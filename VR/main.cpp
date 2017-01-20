#pragma once

#define _CRTDBG_MAP_ALLOC
#include <chrono>
#include <crtdbg.h>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "DxAssert.hpp"
#include "Mesh.hpp"
#include "Profiler.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "VRDevice.hpp"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Create VRDevice.
    VRDevice hmd;

    unsigned int winWidth;
    unsigned int winHeight;
    if (hmd.IsActive())
    {
        winWidth = (hmd.GetRenderWidth());
        winHeight = (hmd.GetRenderHeight() / 2);
    }
    else 
    {
        winWidth = 1024;
        winHeight = 1024;
    }

    // Create renderer.
    Renderer renderer(winWidth, winHeight);

    // Init3D3 (Frame buffers).
    if (hmd.IsActive()) hmd.InitD3D(renderer.mDevice, renderer.mDeviceContext);

    // Create scene.
    Mesh* mesh;
    Scene scene(renderer.mDevice, renderer.mDeviceContext);
    {
        mesh = new Mesh(renderer.mDevice, renderer.mDeviceContext, scene.mStandardMaterial);
        mesh->Load("resources/assets/OBJBox.obj");
        Entity entity;
        entity.mpMesh = mesh;
        {
            int r = 3;
            for (int z = -r; z <= r; ++z)
                for (int y = -r; y <= r; ++y)
                    for (int x = -r; x <= r; ++x)
                    {
                        entity.mPosition = glm::vec3(x, y, z) * 5.f;
                        scene.mEntityList.push_back(entity);
                    }
        }
    }

    // Create camera.
    Camera camera;
    camera.mPosition = glm::vec3(0.f, 0.f, 0.f);

    // Set Frame Latency.
    //IDXGIDevice1 * pDXGIDevice;
    //DxAssert(renderer.mDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice), S_OK);
    //DxAssert(pDXGIDevice->SetMaximumFrameLatency(1), S_OK);
    //pDXGIDevice->Release();

    long long lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    float dt = 0.f;
    while (renderer.Running())
    {
        { PROFILE("FRAME: " + std::to_string(10), true);
            long long newTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            dt = static_cast<float>(newTime - lastTime)/1000.f;
            lastTime = newTime;

            // Clear window.
            renderer.WinClear();

            // VR.
            if (hmd.IsActive())
            {
                renderer.Render(scene, hmd);
            }   
            // Camera.
            else
            {
                camera.Update(20.f, dt, &renderer);
                renderer.Render(scene, camera);
            }

            // Present.
            renderer.WinPresent();

            // Render and update VR pose.
            if (hmd.IsActive())
            {
                renderer.HMDPresent(hmd);
                hmd.Update();
            }
        }
    }


    // --- Shutdown --- //
    {
        // DirectX debug device.
        ID3D11Debug* debug;
        renderer.mDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));

        // Clear.
        delete mesh;
        scene.Clear();
        hmd.Shutdown();
        renderer.Shutdown();

        debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY); //D3D11_RLDO_SUMMARY or D3D11_RLDO_DETAIL
        debug->Release();
    }
        
    return 0;
}
