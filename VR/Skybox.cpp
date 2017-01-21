#include "DxHelp.hpp"
#include "FrameBuffer.hpp"
#include "Skybox.hpp"
#include "Texture2D.hpp"

Skybox::Skybox(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mCubeMapSRV = nullptr;

    DxHelp::CreateVS(mpDevice, "resources/shaders/ScreenQuad_VS.hlsl", &mScreenQuadVS);
    DxHelp::CreatePS(mpDevice, "resources/shaders/Skybox_PS.hlsl", &mSkyboxPS);
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<PSMeta>(mpDevice, 1, &mPSMetaBuff);
}

Skybox::~Skybox()
{
    if (mCubeMapSRV != nullptr) mCubeMapSRV->Release();
    mScreenQuadVS->Release();
    mSkyboxPS->Release();
    mPSMetaBuff->Release();
}

void Skybox::Load(Texture2D* mpBK, Texture2D* mpDN, Texture2D* mpFR, Texture2D* mpLF, Texture2D* mpRT, Texture2D* mpUP)
{
    assert(mCubeMapSRV == nullptr);
    
    ID3D11Texture2D* srcTex[6] = { mpBK->mTex, mpDN->mTex, mpFR->mTex, mpLF->mTex, mpRT->mTex, mpUP->mTex };

    // Assert every texture are the same.
    unsigned int width = mpBK->mWidth;
    unsigned int height = mpBK->mHeight; 
    unsigned int mipLevels = mpBK->mMipLevels;
    DXGI_FORMAT format = mpBK->mFormat;

    ID3D11Texture2D* textureArr;
    {
        D3D11_TEXTURE2D_DESC texDesc;
        ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.ArraySize = 6;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.MipLevels = mipLevels;
        texDesc.Format = format;
        texDesc.CPUAccessFlags = 0;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &textureArr), S_OK);
    }

    // Copy texture (with mips) to texture array.
    D3D11_BOX sourceRegion;
    for (int i = 0; i < 6; ++i)
    {
        for (unsigned int mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
        {
            sourceRegion.left = 0;
            sourceRegion.right = (width >> mipLevel);
            sourceRegion.top = 0;
            sourceRegion.bottom = (height >> mipLevel);
            sourceRegion.front = 0;
            sourceRegion.back = 1;

            //test for overflow
            if (sourceRegion.bottom == 0 || sourceRegion.right == 0)
                break;

            mpDeviceContext->CopySubresourceRegion(textureArr, D3D11CalcSubresource(mipLevel, i, mipLevels), 0, 0, 0, srcTex[i], mipLevel, &sourceRegion);
        }
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    srvDesc.Format = format;
    srvDesc.TextureCube.MipLevels = mipLevels;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

    DxAssert(mpDevice->CreateShaderResourceView(textureArr, &srvDesc, &mCubeMapSRV), S_OK);
    textureArr->Release();
}

void Skybox::Render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, FrameBuffer* targetFb)
{
    mpDeviceContext->OMSetRenderTargets(1, &targetFb->mColRTV, nullptr);
    mpDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    mpDeviceContext->VSSetShader(mScreenQuadVS, nullptr, 0);
    mpDeviceContext->PSSetShader(mSkyboxPS, nullptr, 0);
    mpDeviceContext->PSSetShaderResources(0, 1, &mCubeMapSRV);
    mPSMeta.projectionMatrix = glm::transpose(glm::inverse(projectionMatrix));
    mPSMeta.viewMatrix = glm::transpose(glm::inverse(viewMatrix));
    DxHelp::WriteStructuredBuffer<PSMeta>(mpDeviceContext, &mPSMeta, 1, mPSMetaBuff);
    mpDeviceContext->PSSetShaderResources(1, 1, &mPSMetaBuff);
    D3D11_VIEWPORT vp;
    vp.Width = (float)targetFb->mWidth;
    vp.Height = (float)targetFb->mHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    mpDeviceContext->RSSetViewports(1, &vp);

    mpDeviceContext->Draw(4, 0);

    void* p[1] = { NULL };
    mpDeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)p, nullptr);
    mpDeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mpDeviceContext->VSSetShader(NULL, nullptr, 0);
    mpDeviceContext->PSSetShader(NULL, nullptr, 0);
}
