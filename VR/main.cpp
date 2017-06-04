#pragma once

#define CAMERA_CONTROLL 0
//#define D3D_REPORT_LIVE_OBJ
#define _CRTDBG_MAP_ALLOC
//#define BUILD_VR

#include <chrono>
#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>

#include "Camera.hpp"
#include "CPUTimer.hpp"
#include "D3D11Timer.hpp"
#include "DoubleFrameBuffer.hpp"
#include "DxAssert.hpp"
#include "InputManager.hpp"
#include "Mesh.hpp"
#include "Renderer.hpp"
#include "RenderSystem.hpp"
#include "Scene.hpp"
#include "Skybox.hpp"
#include "Particle/ParticleEmitter.hpp"
#include "Particle/ParticleSystem.hpp"
#include "Texture2D.hpp"
#include "VRDevice.hpp"

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // +++ INIT DEVICES +++ //
    // Create VRDevice.
    VRDevice hmd;
    bool VR = false;
#ifdef BUILD_VR
    VR = hmd.Start();
#endif
    unsigned int winWidth;
    unsigned int winHeight;
    if (VR)
    {
        winWidth = (hmd.GetWidth());
        winHeight = (hmd.GetHeight()/2);
    }
    else 
    {
        winWidth = 1920 / 2;
        winHeight = 1080 / 2;
    }
    // Init D3D devices.
    Renderer renderer(winWidth, winHeight);
    ID3D11Device* pDevice = renderer.mDevice;
    ID3D11DeviceContext* pDeviceContext = renderer.mDeviceContext;
	DoubleFrameBuffer cameraFrameBuffer(pDevice, pDeviceContext, winWidth, winHeight, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    DoubleFrameBuffer hmdLeftFrameBuffer(pDevice, pDeviceContext, winWidth, winHeight, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    DoubleFrameBuffer hmdRightFrameBuffer(pDevice, pDeviceContext, winWidth, winHeight, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    if (VR)
    {
        hmd.InitD3D(pDevice, pDeviceContext, &hmdLeftFrameBuffer, &hmdRightFrameBuffer);
    }
    // --- INIT DEVICES --- //

    // +++ INIT SCENE +++ //
    Skybox skybox(pDevice, pDeviceContext);
    {
        Texture2D bk(pDevice, pDeviceContext);
        bk.Load("../resources/assets/DeepSpaceBlue/backImage.png");
        Texture2D dn(pDevice, pDeviceContext);
        dn.Load("../resources/assets/DeepSpaceBlue/downImage.png");
        Texture2D fr(pDevice, pDeviceContext);
        fr.Load("../resources/assets/DeepSpaceBlue/frontImage.png");
        Texture2D lf(pDevice, pDeviceContext);
        lf.Load("../resources/assets/DeepSpaceBlue/leftImage.png");
        Texture2D rt(pDevice,pDeviceContext);
        rt.Load("../resources/assets/DeepSpaceBlue/rightImage.png");
        Texture2D up(pDevice, pDeviceContext);
        up.Load("../resources/assets/DeepSpaceBlue/upImage.png");
        skybox.Load(&bk, &dn, &fr, &lf, &rt, &up);
    }
    Camera camera(60.f, &cameraFrameBuffer);
	camera.mPosition = glm::vec3(0,0,-2.f);

    RenderSystem renderSystem(pDevice, pDeviceContext);
    ParticleSystem particleSystem(pDevice, pDeviceContext);
    
    InputManager inputManager(renderer.mGLFWwindow);
    
    Mesh mesh(pDevice, pDeviceContext);
    Texture2D albedo(pDevice, pDeviceContext);
    Texture2D normal(pDevice, pDeviceContext);
	Texture2D white(pDevice, pDeviceContext);
	Texture2D black(pDevice, pDeviceContext);
	Texture2D whiteBlack(pDevice, pDeviceContext);

    ParticleEmitter particleEmitter(pDevice, pDeviceContext, 100);
    Scene scene(pDevice, pDeviceContext);
    {
        scene.mpSkybox = &skybox;
        mesh.Load("../resources/assets/skull/skull.obj");
		//mesh.Load("../resources/assets/OBJBox.obj");
		albedo.Load("../resources/assets/skull/skull_diffuse1.jpg");
		//diffuse.Load("../resources/assets/DefaultDiffuse.png");
		normal.Load("../resources/assets/skull/skull_normal.jpg");
		//normal.Load("../resources/assets/DefaultNormal.png");
		white.Load("../resources/assets/White.png");
		black.Load("../resources/assets/Black.png");
		whiteBlack.Load("../resources/assets/WhiteBlack.jpg");

        {   // MESHES
            Entity entity;
            entity.mpMesh = &mesh;
            entity.mpAlbedoTex = &albedo;
            entity.mpNormalTex = &normal;
            {
                int r = 2;
                for (int z = 0; z < r; ++z)
                {
                    for (int y = 0; y < r; ++y)
                    {
                        for (int x = 0; x < r; ++x)
                        {
                            entity.mPosition = glm::vec3(x, y, z) * 10.f;
                            if (x % 2) entity.mpMetalTex = &white;
                            else entity.mpMetalTex = &black;
                            if (y % 2) entity.mpGlossTex = &white;
                            else entity.mpGlossTex = &black;
                            if (z % 2) entity.mTransparent = true;
                            else entity.mTransparent = false;
                            scene.mEntityList.push_back(entity);
                        }
                    }
                }
            }
        }

        {
            Entity entity;
            entity.mpParticleEmitter = &particleEmitter;
            scene.mEntityList.push_back(entity);
        }
        
    }
    // --- INIT SCENE --- //

    // +++ MAIN LOOP +++ //
    float dt = 0.f;
    D3D11Timer gpuTimer(pDevice, pDeviceContext);
    while (renderer.Running())
    {
        // +++ PRE FRAME +++ //
        CPUTIMER(dt);
        bool cpuProfile = inputManager.KeyPressed(GLFW_KEY_F1);
        bool gpuProfile = inputManager.KeyPressed(GLFW_KEY_F2);
        if (gpuProfile) gpuTimer.Start();
        // --- PRE FRAME --- //

        // +++ PRE RENDER UPDATE +++ //
        particleSystem.Update(scene, dt);

        if (VR)
        {
            hmd.mPosition += hmd.mFrontDirection * dt * 10.f;
            scene.SortBackToFront(hmd.mPosition, hmd.mFrontDirection);
        }
        else
        {
            camera.Update(20.f, 2.f, dt, &inputManager);
            scene.SortBackToFront(camera.mPosition, camera.mFrontDirection);
        }
        // --- PRE RENDER UPDATE --- //

        // +++ RENDER +++ //
        renderer.WinClear();
        if (VR)
        {
            hmd.ClearFrameBuffers();
            renderSystem.Render(scene, hmd);
            if (CAMERA_CONTROLL)
                renderSystem.Render(scene, camera);
            else
                renderer.RenderCompanionWindow(hmd.mpLeftFrameBuffer->GetFrameBuffer(), hmd.mpRightFrameBuffer->GetFrameBuffer(), renderer.mWinFrameBuffer); //TODO make own class.
        }
        else
        {
			camera.mpFrameBuffer->Clear(0.f, 1.f, 0.f, 1.f, 1.f);
            renderSystem.Render(scene, camera);
            particleSystem.Render(scene, camera);
        }
        // --- RENDER --- //

        // +++ POST RENDER UPDATE +++ //
        // --- POST RENDER UPDATE --- //

		// +++ PRESENET +++ //
        renderer.WinPresent(camera.mpFrameBuffer->GetFrameBuffer());
        if (VR)
        {
            hmd.Submit();
            if (CAMERA_CONTROLL)
                hmd.mPosition = camera.mPosition;
            hmd.Sync();
        }
        // --- PRESENET --- //

        // +++ POST FRAME +++ //
        if (gpuProfile) gpuTimer.Stop();
        if (gpuProfile) std::cout << "GPU: " << gpuTimer.GetDeltaTime() / 1000000.f << " ms." << std::endl;
        if (cpuProfile) std::cout << "CPU: " << dt * 1000.f << " ms." << std::endl;
        // --- POST FRAME --- //
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
