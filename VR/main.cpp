#pragma once

#define _CRTDBG_MAP_ALLOC
#include <chrono>
#include <crtdbg.h>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "DxAssert.hpp"
#include "Profiler.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "VRDevice.hpp"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Create renderer.
    Renderer renderer(1024, 1024);

    // Create VRDevice.
    VRDevice hmd(renderer.mDevice, renderer.mDeviceContext);

    // Create scene.
    Scene scene(renderer.mDevice, renderer.mDeviceContext);

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

    return 0;
}
