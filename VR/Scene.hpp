#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <glm/vec3.hpp>
#include <vector>
#include "Entity.hpp"

class Material;
class Skybox;

class Scene
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        Scene(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~Scene();

        // Sort scene back to front.
        // cameraPosition Position of camera.
        // cameraFrontDirection Front direction of camera.
        void SortBackToFront(const glm::vec3& cameraPosition, const glm::vec3& cameraFrontDirection);

        // List of entities in scene.
        std::vector<Entity> mEntityList;

        // Skybox.
        Skybox* mpSkybox;

    private:
        void mQuickSort(Entity* arr, int left, int right, const glm::vec3& cameraPosition, const glm::vec3& cameraFrontDirection);
        // i < j
        bool mCompare(const Entity& i, const Entity& j, const glm::vec3& cameraPosition, const glm::vec3& cameraFrontDirection);

        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
