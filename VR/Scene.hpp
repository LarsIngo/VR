#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
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

        // Clear scene.
        void Clear();

        // Standard material.
        Material* mStandardMaterial;

        // List of entities in scene.
        std::vector<Entity> mEntityList;

        // Skybox.
        Skybox* mpSkybox;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
