#pragma once

#define _CRTDBG_MAP_ALLOC
#include <chrono>
#include <crtdbg.h>
#include <glm/glm.hpp>

#include "DxAssert.hpp"
#include "Profiler.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"

int main() 
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Create renderer.
    Renderer renderer(1024, 1024);

    // Create scene.
    Scene scene(renderer.mDevice, renderer.mDeviceContext);
    Camera& camera = scene.mCamera;
    camera.mPosition = glm::vec3(0.f, 0.f, -5.f);

    // Set Frame Latency.
    IDXGIDevice1 * pDXGIDevice;
    DxAssert(renderer.mDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice), S_OK);
    DxAssert(pDXGIDevice->SetMaximumFrameLatency(1), S_OK);
    pDXGIDevice->Release();

    long long lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    float dt = 0.f;
    while (renderer.Running()) {
        { PROFILE("FRAME: " + std::to_string(10), true);
            long long newTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            dt = static_cast<float>(newTime - lastTime)/1000.f;
            lastTime = newTime;

            // Camera.
            camera.Update(20.f, dt, &renderer);
        
            // Renderer.
            renderer.Render(scene);
        }
    }

    return 0;
}
