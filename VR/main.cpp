#pragma once

#define CAMERA_CONTROLL 0

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
#include "Texture2D.hpp"
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
    Texture2D* diffuse;
    Texture2D* normal;
    Scene scene(renderer.mDevice, renderer.mDeviceContext);
    {
        mesh = new Mesh(renderer.mDevice, renderer.mDeviceContext, scene.mStandardMaterial);
        mesh->Load("resources/assets/skull/skull.obj");
        diffuse = new Texture2D(renderer.mDevice, renderer.mDeviceContext);
        diffuse->Load("resources/assets/skull/skull_diffuse1.jpg");
        normal = new Texture2D(renderer.mDevice, renderer.mDeviceContext);
        normal->Load("resources/assets/skull/skull_normal.jpg");

        Entity entity;
        entity.mpMesh = mesh;
        entity.mpDiffuseTex = diffuse;
        entity.mpNormalTex = normal;
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


            // VR device.
            if (hmd.IsActive())
            {
                renderer.Render(scene, hmd);
            }
			
			camera.Update(20.f, dt, &renderer);

            // Window.
            if (!hmd.IsActive() || CAMERA_CONTROLL)
            {
                renderer.Render(scene, camera);
            }
            else if (hmd.IsActive())
            {
                // Render compainion window.
                renderer.RenderCompanionWindow(hmd.mLeftEyeFB, hmd.mRightEyeFB, renderer.mWinFrameBuffer);
            }

            // Present.
            renderer.WinPresent();

            // Render and update VR pose.
            if (hmd.IsActive())
            {
                renderer.HMDPresent(hmd);
                //.mPosition = camera.mPosition;
                hmd.Update();
				hmd.mPosition += hmd.mFrontDir * dt * 10.f;
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
        delete diffuse;
        delete normal;
        scene.Clear();
        hmd.Shutdown();
        renderer.Shutdown();

        debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY); //D3D11_RLDO_SUMMARY or D3D11_RLDO_DETAIL
        debug->Release();
    }
        
    return 0;
}
