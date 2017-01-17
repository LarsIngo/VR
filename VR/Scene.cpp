#include "Scene.hpp"
#include "DxHelp.hpp"

#include <random>
#include <time.h>
#include <vector>

Scene::Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
}

Scene::~Scene()
{

}
