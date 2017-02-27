#pragma once

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

// Window call back procedure.
static LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

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

        // Get key status.
        // vKey Windows virtual key.
        // Returns whether key is pressed or not.
        bool GetKeyPressed(int vKey);

        // Get mouse inside window.
        // Return whether mouse is inside window or not.
        bool GetMouseInsideWindow();

        // Get mouse postion in screen space.
        // Return mouse position relative window upper left coner.
        glm::vec2 GetMousePosition();

        // Get mouse left button status.
        // Return whether mouse left button is pressed or not.
        bool GetMouseLeftButtonPressed();

        // mWinWidth Window width in pixels.
        unsigned int mWinWidth;
        // mWinHeight Window height in pixels.
        unsigned int mWinHeight;

        // Window handle.
        HWND mHWND;
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
        // Initialise HWND(window).
        void InitialiseHWND();

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
