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

    mDepthColTex = nullptr;
    mDepthColSRV = nullptr;
    mDepthColRTV = nullptr;
    mDepthColUAV = nullptr;

    mDepthStencilTex = nullptr;
    mDepthStencilDSV = nullptr;

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

    // Depth color.
    texDesc.Format = DXGI_FORMAT_R32_FLOAT;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mDepthColTex), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) DxAssert(mpDevice->CreateShaderResourceView(mDepthColTex, NULL, &mDepthColSRV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET) DxAssert(mpDevice->CreateRenderTargetView(mDepthColTex, NULL, &mDepthColRTV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) DxAssert(mpDevice->CreateUnorderedAccessView(mDepthColTex, NULL, &mDepthColUAV), S_OK);

    // Depth stencil.
    bool bindSRV = texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE;
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; //DXGI_FORMAT_R32_TYPELESS
    texDesc.BindFlags = bindSRV ? D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_DEPTH_STENCIL;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mDepthStencilTex), S_OK);

    D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
    ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //DXGI_FORMAT_D32_FLOAT
    depthDesc.Flags = 0;
    depthDesc.Texture2D.MipSlice = 0;
    depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DxAssert(mpDevice->CreateDepthStencilView(mDepthStencilTex, &depthDesc, &mDepthStencilDSV), S_OK);
}

FrameBuffer::~FrameBuffer()
{
    if (mColTex != nullptr) mColTex->Release();
    if (mColSRV != nullptr) mColSRV->Release();
    if (mColRTV != nullptr) mColRTV->Release();
    if (mColUAV != nullptr) mColUAV->Release();
    if (mDepthColTex != nullptr) mDepthColTex->Release();
    if (mDepthColSRV != nullptr) mDepthColSRV->Release();
    if (mDepthColRTV != nullptr) mDepthColRTV->Release();
    if (mDepthColUAV != nullptr) mDepthColUAV->Release();
    if (mDepthStencilTex != nullptr) mDepthStencilTex->Release();
    if (mDepthStencilDSV != nullptr) mDepthStencilDSV->Release();
}

void FrameBuffer::ClearAll(float r, float g, float b, float a, float depth)
{
    ClearColor(r, g, b, a);
    ClearDepth(depth);
}

void FrameBuffer::ClearColor(float r, float g, float b, float a)
{
    float clrColor[4] = { r, g, b, a };
    if (mColRTV != nullptr) mpDeviceContext->ClearRenderTargetView(mColRTV, clrColor);
    if (mDepthColRTV != nullptr) mpDeviceContext->ClearRenderTargetView(mDepthColRTV, clrColor);
}

void FrameBuffer::ClearDepth(float depth)
{
    if (mDepthStencilDSV != nullptr) mpDeviceContext->ClearDepthStencilView(mDepthStencilDSV, D3D11_CLEAR_DEPTH, depth, 0);
}

void FrameBuffer::Copy(FrameBuffer* fb)
{
    assert(fb != this);
	assert(mWidth == fb->mWidth && mHeight == fb->mHeight);
	D3D11_BOX sourceRegion;

    // Color texture.
	for (unsigned int mipLevel = 0; mipLevel < mMipLevels; ++mipLevel)
	{
		sourceRegion.left = 0;
		sourceRegion.right = (mWidth >> mipLevel);
		sourceRegion.top = 0;
		sourceRegion.bottom = (mHeight >> mipLevel);
		sourceRegion.front = 0;
		sourceRegion.back = 1;
		assert(sourceRegion.bottom != 0 || sourceRegion.right != 0);

		mpDeviceContext->CopySubresourceRegion(mColTex, D3D11CalcSubresource(mipLevel, 0, mMipLevels), 0, 0, 0, fb->mColTex, mipLevel, &sourceRegion);
        mpDeviceContext->CopySubresourceRegion(mDepthColTex, D3D11CalcSubresource(mipLevel, 0, mMipLevels), 0, 0, 0, fb->mDepthColTex, mipLevel, &sourceRegion);
	}

    // Depth texture.
    mpDeviceContext->CopyResource(mDepthStencilTex, fb->mDepthStencilTex);
}
