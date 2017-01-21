#pragma once

#define CAMERA_CONTROLL 0
//#define FRAME_LATENCY
//#define D3D_REPORT_LIVE_OBJ
#define _CRTDBG_MAP_ALLOC

#include <chrono>
#include <crtdbg.h>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "DxAssert.hpp"
#include "Mesh.hpp"
#include "Profiler.hpp"
#include "Renderer.hpp"
#include "RenderSystem.hpp"
#include "Scene.hpp"
#include "Skybox.hpp"
#include "Texture2D.hpp"
#include "VRDevice.hpp"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // +++ INIT DEVICES +++ //
    // Create VRDevice.
    VRDevice hmd;
    bool VR = hmd.Start();
    unsigned int winWidth;
    unsigned int winHeight;
    if (VR)
    {
        winWidth = (hmd.GetRenderWidth());
        winHeight = (hmd.GetRenderHeight()/2);
    }
    else 
    {
        winWidth = 1024;
        winHeight = 1024;
    }
    // Init D3D devices.
    Renderer renderer(winWidth, winHeight);
#ifdef FRAME_LATENCY
    // Set Frame Latency.
    IDXGIDevice1 * pDXGIDevice;
    DxAssert(renderer.mDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice), S_OK);
    DxAssert(pDXGIDevice->SetMaximumFrameLatency(1), S_OK);
    pDXGIDevice->Release();
#endif
    FrameBuffer hmdLeftFrameBuffer(renderer.mDevice, renderer.mDeviceContext, winWidth, winHeight, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    FrameBuffer hmdRightFrameBuffer(renderer.mDevice, renderer.mDeviceContext, winWidth, winHeight, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    if (VR)
    {
        hmd.InitD3D(renderer.mDevice, renderer.mDeviceContext, &hmdLeftFrameBuffer, &hmdRightFrameBuffer);
    }
    // --- INIT DEVICES --- //

    // +++ INIT SCENE +++ //
    Skybox skybox(renderer.mDevice, renderer.mDeviceContext);
    {
        Texture2D bk(renderer.mDevice, renderer.mDeviceContext);
        bk.Load("resources/assets/DeepSpaceBlue/backImage.png");
        Texture2D dn(renderer.mDevice, renderer.mDeviceContext);
        dn.Load("resources/assets/DeepSpaceBlue/downImage.png");
        Texture2D fr(renderer.mDevice, renderer.mDeviceContext);
        fr.Load("resources/assets/DeepSpaceBlue/frontImage.png");
        Texture2D lf(renderer.mDevice, renderer.mDeviceContext);
        lf.Load("resources/assets/DeepSpaceBlue/leftImage.png");
        Texture2D rt(renderer.mDevice, renderer.mDeviceContext);
        rt.Load("resources/assets/DeepSpaceBlue/rightImage.png");
        Texture2D up(renderer.mDevice, renderer.mDeviceContext);
        up.Load("resources/assets/DeepSpaceBlue/upImage.png");
        skybox.Load(&bk, &dn, &fr, &lf, &rt, &up);
    }
    Camera camera(winWidth, winHeight, renderer.mWinFrameBuffer); camera.mPosition.z = 1;
    RenderSystem renderSystem(renderer.mDevice, renderer.mDeviceContext);
    Mesh mesh(renderer.mDevice, renderer.mDeviceContext);
    Texture2D diffuse(renderer.mDevice, renderer.mDeviceContext);
    Texture2D normal(renderer.mDevice, renderer.mDeviceContext);
    Scene scene(renderer.mDevice, renderer.mDeviceContext);
    {
        scene.mpSkybox = &skybox;
        mesh.Load("resources/assets/skull/skull.obj");
        diffuse.Load("resources/assets/skull/skull_diffuse1.jpg");
        normal.Load("resources/assets/skull/skull_normal.jpg");

        Entity entity;
        entity.mpMesh = &mesh;
        entity.mpDiffuseTex = &diffuse;
        entity.mpNormalTex = &normal;
        {
            int r = 2;
            for (int z = -r; z <= r; ++z)
                for (int y = -r; y <= r; ++y)
                    for (int x = -r; x <= r; ++x)
                    {
                        entity.mPosition = glm::vec3(x, y, z) * 5.f;
                        scene.mEntityList.push_back(entity);
                    }
        }
    }
    // --- INIT SCENE --- //

    // +++ MAIN LOOP +++ //
    long long lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    float dt = 0.f;
    while (renderer.Running())
    {
        { PROFILE("FRAME: " + std::to_string(10), true);
            long long newTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            dt = static_cast<float>(newTime - lastTime)/1000.f;
            lastTime = newTime;

            // +++ UPDATE +++ //
            if (VR)
                hmd.mPosition += hmd.mFrontDirection * dt * 10.f;
            camera.Update(20.f, dt, &renderer);
            // --- UPDATE --- //

            // +++ RENDER +++ //
            renderer.WinClear();
            if (VR)
            {
                hmd.ClearFrameBuffers();
                renderSystem.Render(scene, hmd);
                if (CAMERA_CONTROLL)
                {
                    renderSystem.Render(scene, camera);
                }
                else
                {
                    renderer.RenderCompanionWindow(hmd.mpLeftFrameBuffer, hmd.mpRightFrameBuffer, renderer.mWinFrameBuffer); //TODO make own class.
                }
                    
            }
            else
            {
                renderSystem.Render(scene, camera);
            }
            // --- RENDER --- //

            // +++ PRESENET +++ //
            renderer.WinPresent();
            if (VR)
            {
                hmd.Submit();
                if (CAMERA_CONTROLL)
                    hmd.mPosition = camera.mPosition;
                hmd.Sync();
            }
            // --- PRESENET --- //
        }
    }
    // --- MAIN LOOP --- //

    // +++ SHUTDOWN +++ //

#ifdef D3D_REPORT_LIVE_OBJ
    ID3D11Debug* debug;
    renderer.mDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
    debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY); //D3D11_RLDO_SUMMARY or D3D11_RLDO_DETAIL
    debug->Release();
#endif
    // --- SHUTDOWN --- //
    return 0;
}
