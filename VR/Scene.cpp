#include "Scene.hpp"
#include "DxHelp.hpp"
#include "Material.hpp"

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mpSkybox = nullptr;

    mStandardMaterial = new Material(mpDevice, mpDeviceContext);
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    mStandardMaterial->Init(inputDesc, "resources/shaders/Standard_VS.hlsl", "resources/shaders/Standard_GS.hlsl", "resources/shaders/Standard_PS.hlsl");
}

Scene::~Scene()
{
    Clear();
}

void Scene::Clear()
{
    if (mStandardMaterial != nullptr) { delete mStandardMaterial; mStandardMaterial = nullptr; }
}
