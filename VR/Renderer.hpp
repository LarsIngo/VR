#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>

#include <glm/vec2.hpp>
#include <map>
#include <string>
#include <vector>
#include <Windows.h>

#include "Camera.hpp"
#include "Scene.hpp"
#include "VRDevice.hpp"
#include "Material.hpp"

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

        // Whether window is running of not.
        bool Running() const;

        // Close window.
        void Close();

        // Render scene.
        // scene Scene to render.
        // camera Camera to render from.
        void Render(Scene& scene, Camera& camera) const;
        // scene Scene to render.
        // hmd VrDevice to render from.
        void Render(Scene& scene, VRDevice& hmd) const;

        // Present rendered result to window.
        void WinPresent();
        // Submit rendered result to HMD.
        void HMDPresent(VRDevice& hmd);

        // Render to render target view.
        // scene Scene to render.
        // material Material to use.
        // rtv Render target.
        void RenderRTV(Scene& scene, Material* material, ID3D11RenderTargetView* rtv) const;

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

        // Backbuffer texture.
        ID3D11Texture2D* mBackBufferTex;
        // Backbuffer render target view.
        ID3D11RenderTargetView* mBackBufferRTV;

        // Screen quad vertex shader.
        ID3D11VertexShader* mScreenQuadVS;
        // Companion window pixel shader.
        ID3D11PixelShader* mCompanionWindowPS;

    private:
        // Initialise HWND(window).
        void InitialiseHWND();

        // Initialise D3D(directX).
        void InitialiseD3D();

        // Render companion window.
        void RenderCompanionWindow(ID3D11ShaderResourceView* leftEye, ID3D11ShaderResourceView* RightEye, ID3D11RenderTargetView* rtv) const;

        // Mouse position.
        glm::vec2 mMousePosition;

        // Window should close.
        bool mClose;
};
