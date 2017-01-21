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
    mGSMetaBuff = nullptr;
}

Material::~Material()
{
    if (mInputLayout != nullptr) mInputLayout->Release();
    if (mVS != nullptr) mVS->Release();
    if (mGS != nullptr) mGS->Release();
    if (mPS != nullptr) mPS->Release();
    if (mGSMetaBuff != nullptr) mGSMetaBuff->Release();
}

void Material::Init(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const char* VSPath, const char* GSPath, const char* PSPath)
{
    DxHelp::CreateVS(mpDevice, VSPath, &mVS, &inputDesc, &mInputLayout);
    DxHelp::CreateGS(mpDevice, GSPath, &mGS);
    DxHelp::CreatePS(mpDevice, PSPath, &mPS);
    DxHelp::CreateCPUwriteGPUreadStructuredBuffer<GSMeta>(mpDevice, 1, &mGSMetaBuff);
}
