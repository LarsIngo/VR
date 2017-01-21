#include "Scene.hpp"
#include "DxHelp.hpp"
#include "Material.hpp"

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mpSkybox = nullptr;
}

Scene::~Scene()
{

}
