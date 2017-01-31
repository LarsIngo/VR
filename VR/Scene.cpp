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

void Scene::SortBackToFront(const glm::vec3& cameraPosition, const glm::vec3& cameraFrontDirection)
{
    if (mEntityList.size() > 1) mQuickSort(mEntityList.data(), 0, mEntityList.size() - 1, cameraPosition, cameraFrontDirection);
}

void Scene::mQuickSort(Entity* arr, int left, int right, const glm::vec3& cameraPosition, const glm::vec3& cameraFrontDirection)
{
    /* http://www.algolist.net/Algorithms/Sorting/Quicksort */
    int i = left, j = right;
    Entity tmp;
    Entity pivot = arr[(left + right) / 2];

    /* partition */
    while (i <= j) {
        while (mCompare(arr[i], pivot, cameraPosition, cameraFrontDirection))
            i++;
        while (mCompare(pivot, arr[j], cameraPosition, cameraFrontDirection))
            j--;
        if (i <= j) {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    };

    /* recursion */
    if (left < j)
        mQuickSort(arr, left, j, cameraPosition, cameraFrontDirection);
    if (i < right)
        mQuickSort(arr, i, right, cameraPosition, cameraFrontDirection);
}

bool Scene::mCompare(const Entity& i, const Entity& j, const glm::vec3& cameraPosition, const glm::vec3& cameraFrontDirection)
{
    return glm::dot(i.mPosition - cameraPosition, cameraFrontDirection) > glm::dot(j.mPosition - cameraPosition, cameraFrontDirection);
    //return i.mPosition.x < j.mPosition.x;
}
