#pragma once

#ifdef _DEBUG
#define BUILD_ENABLE_D3D11_DEBUG
#endif

#include <GLFW/glfw3.h>
#include <d3d11.h>
#include <d3dcompiler.inl>

#include <glm/vec2.hpp>
#include <map>
#include <string>
#include <vector>
#include <Windows.h>

#include "Camera.hpp"
#include "FrameBuffer.hpp"
#include "Scene.hpp"
#include "VRDevice.hpp"

class Material;

// Renders scene to windows.
class Renderer 
{
    public:
        // Constructor.
        // winWidth Window width in pixels.
        // winHeight Window height in pixels.
        Renderer(unsigned int winWidth = 640, unsigned int winHeight = 640);

        // Destructor.
        ~Renderer();

        // Shutdown.
        void Shutdown();

        // Whether window is running of not.
        bool Running() const;

        // Close window.
        void Close();

        // Clear backbuffer.
        void WinClear();
        // Present rendered result to window.
        void WinPresent(FrameBuffer* fb);

        // Render companion window.
        void RenderCompanionWindow(FrameBuffer* leftEyeFb, FrameBuffer* rightEyeFb, FrameBuffer* windowFb);

        // mWinWidth Window width in pixels.
        unsigned int mWinWidth;
        // mWinHeight Window height in pixels.
        unsigned int mWinHeight;

        // GLFW window.
        GLFWwindow* mGLFWwindow;

        // Window swap chain.
        IDXGISwapChain* mSwapChain;

        // DirectX device.
        ID3D11Device* mDevice;
        // DirectX device context.
        ID3D11DeviceContext* mDeviceContext;

        // Window frame buffer.
        FrameBuffer* mWinFrameBuffer;

        // Screen quad vertex shader.
        ID3D11VertexShader* mScreenQuadVS;
        // Companion window pixel shader.
        ID3D11PixelShader* mCompanionWindowPS;

    private:
        // Initialise Window.
        void InitialiseGLFW();

        // Initialise D3D(directX).
        void InitialiseD3D();

        // Mouse position.
        glm::vec2 mMousePosition;

        // Window should close.
        bool mClose;

        // Sampler.
        ID3D11SamplerState* mSampState;

        // Rasterizer.
        ID3D11RasterizerState* mRasState;
};
