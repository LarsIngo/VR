#include "Scene.hpp"
#include "DxHelp.hpp"
#include "Material.hpp"

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mStandardMaterial = new Material(mpDevice, mpDeviceContext);
    std::wstring VSPath = L"resources/shaders/Standard_VS.hlsl";
    std::wstring GSPath = L"resources/shaders/Standard_GS.hlsl";
    std::wstring PSPath = L"resources/shaders/Standard_PS.hlsl";
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    mStandardMaterial->Init(inputDesc, VSPath, GSPath, PSPath);
}

Scene::~Scene()
{
    delete mStandardMaterial;
}
