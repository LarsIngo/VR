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
    mDepthTex = nullptr;
    mDepthDSV = nullptr;

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
    if (initTexture == nullptr) { DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mColTex), S_OK); }   
    else mColTex = initTexture;
    if (texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) DxAssert(mpDevice->CreateShaderResourceView(mColTex, NULL, &mColSRV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET) DxAssert(mpDevice->CreateRenderTargetView(mColTex, NULL, &mColRTV), S_OK);
    if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) DxAssert(mpDevice->CreateUnorderedAccessView(mColTex, NULL, &mColUAV), S_OK);

    // Depth.
    texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DxAssert(mpDevice->CreateTexture2D(&texDesc, NULL, &mDepthTex), S_OK);
    DxAssert(mpDevice->CreateDepthStencilView(mDepthTex, NULL, &mDepthDSV), S_OK);

	// Generate mip levels.
	if (miscFlags == D3D11_RESOURCE_MISC_GENERATE_MIPS)
		mpDeviceContext->GenerateMips(mColSRV);

	// Get desc info.
	D3D11_TEXTURE2D_DESC desc;
	mColTex->GetDesc(&desc);
	mMipLevels = desc.MipLevels;
}

FrameBuffer::~FrameBuffer()
{
    if (mColTex != nullptr) mColTex->Release();
    if (mColSRV != nullptr) mColSRV->Release();
    if (mColRTV != nullptr) mColRTV->Release();
    if (mColUAV != nullptr) mColUAV->Release();
    if (mDepthTex != nullptr) mDepthTex->Release();
    if (mDepthDSV != nullptr) mDepthDSV->Release();
}

void FrameBuffer::Clear(float r, float g, float b, float a)
{
    float clrColor[4] = { r, g, b, a};
    if (mColRTV != nullptr) mpDeviceContext->ClearRenderTargetView(mColRTV, clrColor);
    if (mDepthDSV != nullptr) mpDeviceContext->ClearDepthStencilView(mDepthDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void FrameBuffer::Copy(FrameBuffer* fb)
{
	assert(mWidth == fb->mWidth && mHeight == fb->mHeight);
	D3D11_BOX sourceRegion;
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
	}
	//sourceRegion.left = 0; sourceRegion.right = mWidth;
	//sourceRegion.top = 0; sourceRegion.bottom = mHeight;
	//sourceRegion.front = 0; sourceRegion.back = 1;
	//mpDeviceContext->CopySubresourceRegion(this->mColTex, 0, 0, 0, 0, fb->mColTex, 0, &sourceRegion);
}
