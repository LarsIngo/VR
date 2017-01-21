#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>

#include "Camera.hpp"
#include "Scene.hpp"
#include "VRDevice.hpp"

class Material;

class RenderSystem
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        RenderSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~RenderSystem();

        // Render.
        void Render(Scene& scene, Camera& camera);
        void Render(Scene& scene, VRDevice& hmd);

        // Material
        Material* mStandardMaterial;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
