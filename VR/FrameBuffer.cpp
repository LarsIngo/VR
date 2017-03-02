#include "FrameBuffer.hpp"
#include "DxHelp.hpp"

FrameBuffer::FrameBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int width, unsigned int height, UINT bindFlags, UINT miscFlags, ID3D11Texture2D* initTexture)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mWidth = width;
    mHeight = height;

    mColTex = nullptr;
    mColSRV = nullptr;
    mColRTV = nullptr;
    mColUAV = nullptr;

    mWorldTex = nullptr;
    mWorldSRV = nullptr;
    mWorldRTV = nullptr;
    mWorldUAV = nullptr;

    mNormTex = nullptr;
    mNormSRV = nullptr;
    mNormRTV = nullptr;
    mNormUAV = nullptr;

    mDepthTex = nullptr;
    mDepthSRV = nullptr;
    mDepthRTV = nullptr;
    mDepthUAV = nullptr;

    mDepthStencilTex = nullptr;
    mDepthStencilDSV = nullptr;

    mStagingTexR32 = nullptr;
    mStagingTexR32G32B32A32 = nullptr;

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Width = mWidth;
    texDesc.Height = mHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = miscFlags;

    // Color.
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags = bindFlags;
    if (initTexture == nullptr) { DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mColTex), S_OK); } else mColTex = initTexture;
    if (texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) DxAssert(mpDevice->CreateShaderResourceView(mColTex, NULL, &mColSRV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET) DxAssert(mpDevice->CreateRenderTargetView(mColTex, NULL, &mColRTV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) DxAssert(mpDevice->CreateUnorderedAccessView(mColTex, NULL, &mColUAV), S_OK);
    // Generate mip levels.
    if (miscFlags == D3D11_RESOURCE_MISC_GENERATE_MIPS)
        mpDeviceContext->GenerateMips(mColSRV);
    // Get desc info.
    D3D11_TEXTURE2D_DESC desc;
    mColTex->GetDesc(&desc);
    mMipLevels = desc.MipLevels;


    // World.
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mWorldTex), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) DxAssert(mpDevice->CreateShaderResourceView(mWorldTex, NULL, &mWorldSRV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET) DxAssert(mpDevice->CreateRenderTargetView(mWorldTex, NULL, &mWorldRTV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) DxAssert(mpDevice->CreateUnorderedAccessView(mWorldTex, NULL, &mWorldUAV), S_OK);


    // Normal.
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mNormTex), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) DxAssert(mpDevice->CreateShaderResourceView(mNormTex, NULL, &mNormSRV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET) DxAssert(mpDevice->CreateRenderTargetView(mNormTex, NULL, &mNormRTV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) DxAssert(mpDevice->CreateUnorderedAccessView(mNormTex, NULL, &mNormUAV), S_OK);


    // Depth.
    texDesc.Format = DXGI_FORMAT_R32_FLOAT;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mDepthTex), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) DxAssert(mpDevice->CreateShaderResourceView(mDepthTex, NULL, &mDepthSRV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET) DxAssert(mpDevice->CreateRenderTargetView(mDepthTex, NULL, &mDepthRTV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) DxAssert(mpDevice->CreateUnorderedAccessView(mDepthTex, NULL, &mDepthUAV), S_OK);


    // Depth stencil.
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.BindFlags = texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE ? D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_DEPTH_STENCIL;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mDepthStencilTex), S_OK);
    D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
    ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.Flags = 0;
    depthDesc.Texture2D.MipSlice = 0;
    depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DxAssert(mpDevice->CreateDepthStencilView(mDepthStencilTex, &depthDesc, &mDepthStencilDSV), S_OK);


    // Staging buffers.
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texDesc.BindFlags = 0;
    texDesc.Format = DXGI_FORMAT_R32_FLOAT;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mStagingTexR32), S_OK);
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mStagingTexR32G32B32A32), S_OK);
}

FrameBuffer::~FrameBuffer()
{
    if (mColTex != nullptr) mColTex->Release();
    if (mColSRV != nullptr) mColSRV->Release();
    if (mColRTV != nullptr) mColRTV->Release();
    if (mColUAV != nullptr) mColUAV->Release();

    if (mWorldTex != nullptr) mWorldTex->Release();
    if (mWorldSRV != nullptr) mWorldSRV->Release();
    if (mWorldRTV != nullptr) mWorldRTV->Release();
    if (mWorldUAV != nullptr) mWorldUAV->Release();

    if (mNormTex != nullptr) mNormTex->Release();
    if (mNormSRV != nullptr) mNormSRV->Release();
    if (mNormRTV != nullptr) mNormRTV->Release();
    if (mNormUAV != nullptr) mNormUAV->Release();

    if (mDepthTex != nullptr) mDepthTex->Release();
    if (mDepthSRV != nullptr) mDepthSRV->Release();
    if (mDepthRTV != nullptr) mDepthRTV->Release();
    if (mDepthUAV != nullptr) mDepthUAV->Release();

    if (mDepthStencilTex != nullptr) mDepthStencilTex->Release();
    if (mDepthStencilDSV != nullptr) mDepthStencilDSV->Release();

    if (mStagingTexR32 != nullptr) mStagingTexR32->Release();
    if (mStagingTexR32G32B32A32 != nullptr) mStagingTexR32G32B32A32->Release();
}

void FrameBuffer::ClearAll(float r, float g, float b, float a, float depth)
{
    float clrColor[4] = { r, g, b, a };
    float clrWorld[4] = { 0.f, 0.f, 0.f, 0.f };
    float clrNorm[4] = { 0.f, 0.f, 0.f, 0.f };
    float clrDepth[4] = { 0.f, 0.f, 0.f, 0.f };
    if (mColRTV != nullptr) mpDeviceContext->ClearRenderTargetView(mColRTV, clrColor);
    if (mWorldRTV != nullptr) mpDeviceContext->ClearRenderTargetView(mWorldRTV, clrWorld);
    if (mNormRTV != nullptr) mpDeviceContext->ClearRenderTargetView(mNormRTV, clrNorm);
    if (mDepthRTV != nullptr) mpDeviceContext->ClearRenderTargetView(mDepthRTV, clrDepth);
    if (mDepthStencilDSV != nullptr) mpDeviceContext->ClearDepthStencilView(mDepthStencilDSV, D3D11_CLEAR_DEPTH, depth, 0);
}

void FrameBuffer::Copy(FrameBuffer* fb)
{
    assert(fb != this);
	assert(mWidth == fb->mWidth && mHeight == fb->mHeight);
    assert(mMipLevels == fb->mMipLevels);

    DxHelp::CopyTexture(mpDeviceContext, mColTex, fb->mColTex, mWidth, mHeight, mMipLevels);
    DxHelp::CopyTexture(mpDeviceContext, mWorldTex, fb->mWorldTex, mWidth, mHeight, mMipLevels);
    DxHelp::CopyTexture(mpDeviceContext, mNormTex, fb->mNormTex, mWidth, mHeight, mMipLevels);
    DxHelp::CopyTexture(mpDeviceContext, mDepthTex, fb->mDepthTex, mWidth, mHeight, mMipLevels);

    mpDeviceContext->CopyResource(mDepthStencilTex, fb->mDepthStencilTex);
}

glm::vec4* FrameBuffer::ReadWorld()
{
    DxHelp::CopyTexture(mpDeviceContext, mStagingTexR32G32B32A32, mWorldTex, mWidth, mHeight, mMipLevels);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    DxAssert(mpDeviceContext->Map(mStagingTexR32G32B32A32, 0, D3D11_MAP_READ, 0, &mappedResource), S_OK);
    mpDeviceContext->Unmap(mStagingTexR32G32B32A32, 0);

    return (glm::vec4*)mappedResource.pData;
}

glm::vec4* FrameBuffer::ReadNormal()
{
    DxHelp::CopyTexture(mpDeviceContext, mStagingTexR32G32B32A32, mNormTex, mWidth, mHeight, mMipLevels);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    DxAssert(mpDeviceContext->Map(mStagingTexR32G32B32A32, 0, D3D11_MAP_READ, 0, &mappedResource), S_OK);
    mpDeviceContext->Unmap(mStagingTexR32G32B32A32, 0);

    return (glm::vec4*)mappedResource.pData;
}

float* FrameBuffer::ReadDepth()
{
    DxHelp::CopyTexture(mpDeviceContext, mStagingTexR32, mDepthTex, mWidth, mHeight, mMipLevels);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

    DxAssert(mpDeviceContext->Map(mStagingTexR32, 0, D3D11_MAP_READ, 0, &mappedResource), S_OK);
    mpDeviceContext->Unmap(mStagingTexR32, 0);

    return (float*)mappedResource.pData;
}
