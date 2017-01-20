#include "Material.hpp"
#include "DxHelp.hpp"

Material::Material(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mInputLayout = nullptr;
    mVS = nullptr;
    mGS = nullptr;
    mPS = nullptr;
}

Material::~Material()
{
    if (mInputLayout != nullptr) mInputLayout->Release();
    if (mVS != nullptr) mVS->Release();
    if (mGS != nullptr) mGS->Release();
    if (mPS != nullptr) mPS->Release();
}

void Material::Init(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, std::wstring& VSPath, std::wstring& GSPath, std::wstring& PSPath)
{
    DxHelp::CreateVS(mpDevice, VSPath, &mVS, &inputDesc, &mInputLayout);
    DxHelp::CreateGS(mpDevice, GSPath, &mGS);
    DxHelp::CreatePS(mpDevice, PSPath, &mPS);
}
