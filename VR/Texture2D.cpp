#include "Texture2D.hpp"
#include "DxAssert.hpp"

#include <WICTextureLoader.h>
#include <iostream>

Texture2D::Texture2D(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mTexPath = "";
    mWidth = 0;
    mHeight = 0;
    mTex = nullptr;
    mSRV = nullptr;
}

Texture2D::~Texture2D()
{
    if (mTex != nullptr) mTex->Release();
    if (mSRV != nullptr) mSRV->Release();
}

void Texture2D::Load(const char* texPath)
{
    assert(mTex == nullptr && mSRV == nullptr && mWidth == 0 && mHeight == 0 && mTexPath == "");

    mTexPath = std::string(texPath);
    std::wstring wTexPath(mTexPath.begin(), mTexPath.end());
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    ID3D11Resource* res;
    DxAssert(DirectX::CreateWICTextureFromFileEx(mpDevice, mpDeviceContext, wTexPath.c_str(), 0, texDesc.Usage, texDesc.BindFlags, texDesc.CPUAccessFlags, texDesc.MiscFlags, DirectX::WIC_LOADER_DEFAULT, &res, &mSRV), S_OK);
	res->QueryInterface(&mTex);
    res->Release();
    D3D11_TEXTURE2D_DESC desc;
    mTex->GetDesc(&desc);
    mMipLevels = desc.MipLevels;
    mFormat = desc.Format;
    mWidth = desc.Width;
    mHeight = desc.Height;
}
