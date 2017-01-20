#include "Renderer.hpp"
#include "DxAssert.hpp"
#include "DxHelp.hpp"
#include "Material.hpp"
#include "DxHelp.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

Renderer::Renderer(unsigned int winWidth, unsigned int winHeight)
{
    mWinWidth = winWidth;
    mWinHeight = winHeight;
    mClose = false;

    // Window.
    InitialiseHWND();
    // DirectX.
    InitialiseD3D();

    // Init shader
    std::wstring VS = L"resources/shaders/ScreenQuad_VS.hlsl";
    DxHelp::CreateVS(mDevice, VS, &mScreenQuadVS);
    std::wstring PS = L"resources/shaders/CompanionWindow_PS.hlsl";
    DxHelp::CreatePS(mDevice, PS, &mCompanionWindowPS);
}

Renderer::~Renderer() 
{
    mDevice->Release();
    mDeviceContext->Release();
    mSwapChain->Release();
    mBackBufferRTV->Release();
    mBackBufferTex->Release();
    mScreenQuadVS->Release();
    mCompanionWindowPS->Release();
}

bool Renderer::Running() const 
{
    if (mClose) {
        PostQuitMessage(0);
    }

    MSG windowMsg = { 0 };

    while (windowMsg.message != WM_QUIT)
    {
        if (PeekMessage(&windowMsg, NULL, NULL, NULL, PM_REMOVE))
        {
            TranslateMessage(&windowMsg);

            DispatchMessage(&windowMsg);
        }
        else
        {
            // If there are no more messages to handle, run a frame.
            return true;
        }
    }
    return false;
}

void Renderer::Close()
{
    mClose = true;
}

void Renderer::Render(Scene& scene, Camera& camera) const
{
    // Render from camera.
    float clrColor[4] = { 0.f, 0.2f, 0.f, 0.f };
    mDeviceContext->ClearRenderTargetView(mBackBufferRTV, clrColor);
    Material* material = scene.mStandardMaterial;
    material->mGSMeta.mvpMatrix = glm::perspectiveFovLH(45.f, (float)mWinWidth, (float)mWinHeight, 0.01f, 200.f) * camera.mViewMatrix * glm::translate(glm::mat4(), -camera.mPosition);
    RenderRTV(scene, material, mBackBufferRTV);
}

void Renderer::Render(Scene& scene, VRDevice& hmd) const
{
    {   // Render left eye.
        float clrColor[4] = { 0.2f, 0.0f, 0.f, 0.f };
        mDeviceContext->ClearRenderTargetView(hmd.mHmdLeftRTV, clrColor);
        Material* material = scene.mStandardMaterial;
        material->mGSMeta.mvpMatrix = hmd.mMVPLeft;
        RenderRTV(scene, material, hmd.mHmdLeftRTV);
    }
    {   // Render right eye.
        float clrColor[4] = { 0.f, 0.f, 0.2f, 0.f };
        mDeviceContext->ClearRenderTargetView(hmd.mHmdRightRTV, clrColor);
        Material* material = scene.mStandardMaterial;
        material->mGSMeta.mvpMatrix = hmd.mMVPRight;
        RenderRTV(scene, material, hmd.mHmdRightRTV);
    }
    // Render compainion window.
    RenderCompanionWindow(hmd.mHmdLeftSRV, hmd.mHmdRightSRV, mBackBufferRTV);
}

void Renderer::WinPresent()
{
    // Present to window.
    mSwapChain->Present(0, 0);
}

void Renderer::HMDPresent(VRDevice& hmd)
{
    {   // Submit left eye.
        vr::Texture_t leftEyeTexture = { hmd.mHmdLeftTex, vr::TextureType_DirectX, vr::ColorSpace_Auto };
        vr::EVRCompositorError eError = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
        if (eError != vr::VRCompositorError_None) std::cout << "HMD Error rendering left eye" << std::endl;
    }
    {   // Submit right eye.
        vr::Texture_t rightEyeTexture = { hmd.mHmdRightTex, vr::TextureType_DirectX, vr::ColorSpace_Auto };
        vr::EVRCompositorError eError = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
        if (eError != vr::VRCompositorError_None) std::cout << "HMD Error rendering right eye" << std::endl;
    }
}

void Renderer::RenderRTV(Scene& scene, Material* material, ID3D11RenderTargetView* rtv) const
{
    std::vector<Material::Vertex> vertexArr;
    Material::Vertex vert;

    vert.position = glm::vec3(0.f, 0.f, 1.f);
    vert.normal = glm::vec3(0.f, 0.f, -1.f);
    vert.uv = glm::vec2(0.f, 0.f);
    vertexArr.push_back(vert);

    vert.position = glm::vec3(0.5f, -0.5f, 1.f);
    vert.normal = glm::vec3(0.f, 0.f, -1.f);
    vert.uv = glm::vec2(1.f, 1.f);
    vertexArr.push_back(vert);

    vert.position = glm::vec3(0.f, -0.5f, 1.f);
    vert.normal = glm::vec3(0.f, 0.f, -1.f);
    vert.uv = glm::vec2(0.f, 1.f);
    vertexArr.push_back(vert);

    unsigned int numVertices = vertexArr.size();
    ID3D11Buffer* vertexBuffer;
    DxHelp::CreateVertexBuffer(mDevice, vertexArr.size(), vertexArr.data(), &vertexBuffer);

    // +++ Render +++ //
    material->mGSMeta.modelMatrix = glm::mat4();
    material->mGSMeta.mvpMatrix = glm::transpose(material->mGSMeta.mvpMatrix);
    DxHelp::WriteStructuredBuffer<Material::GSMeta>(mDeviceContext, &material->mGSMeta, 1, material->mGSMetaBuff);
    mDeviceContext->OMSetRenderTargets(1, &rtv, nullptr);
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    unsigned int stride = sizeof(Material::Vertex);
    unsigned int offset = 0;
    mDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    mDeviceContext->GSSetShaderResources(0, 1, &material->mGSMetaBuff);
    mDeviceContext->IASetInputLayout(material->mInputLayout);
    mDeviceContext->VSSetShader(material->mVS, nullptr, 0);
    mDeviceContext->GSSetShader(material->mGS, nullptr, 0);
    mDeviceContext->PSSetShader(material->mPS, nullptr, 0);

    mDeviceContext->Draw(numVertices, 0);

    void* p[1] = { NULL };
    mDeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)p, nullptr);
    mDeviceContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)p, &stride, &offset);
    mDeviceContext->GSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mDeviceContext->VSSetShader(NULL, nullptr, 0);
    mDeviceContext->GSSetShader(NULL, nullptr, 0);
    mDeviceContext->PSSetShader(NULL, nullptr, 0);
    // --- Render --- //

    vertexBuffer->Release();
}

bool Renderer::GetKeyPressed(int vKey)
{
    return GetAsyncKeyState(vKey) != 0 ? true : false;
}

glm::vec2 Renderer::GetMousePosition()
{
    return mMousePosition;
}

bool Renderer::GetMouseInsideWindow()
{
    // Get current mouse position in screen coords.
    POINT pos = { 0, 0 };
    if (GetCursorPos(&pos))
    {
        // Convert position to client window coords.
        if (ScreenToClient(mHWND, &pos))
        {
            // Get window's client rect.
            RECT rcClient = { 0 };
            GetClientRect(mHWND, &rcClient);

            // If mouse cursor is inside rect.
            if (PtInRect(&rcClient, pos)) {
                mMousePosition = glm::vec2(static_cast<float>(pos.x) / mWinWidth * 2.f - 1.f, -(static_cast<float>(pos.y) / mWinHeight * 2.f - 1.f));
                return true;
            }
        }
    }
    return false;
}

bool Renderer::GetMouseLeftButtonPressed()
{
    return (GetKeyState(VK_LBUTTON) & 0x80) != 0;
}

LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // If a message has not been handled, send it to the default window procedure for handling.
    return DefWindowProc(handle, message, wParam, lParam);
}

void Renderer::InitialiseHWND()
{
    // Register the window class to create.
    HINSTANCE applicationHandle = GetModuleHandle(NULL);
    WNDCLASS windowClass;
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = applicationHandle;
    windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = "WindowClass";

    RegisterClass(&windowClass);

    RECT rect = { 0, 0, (long)mWinWidth, (long)mWinHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    mHWND = CreateWindow(
        "WindowClass",
        "Window Title",
        WS_OVERLAPPEDWINDOW,
        10,
        10,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        applicationHandle,
        NULL
    );

    ShowWindow(mHWND, SW_SHOWDEFAULT);
    UpdateWindow(mHWND);
}

void Renderer::InitialiseD3D()
{
    assert(mHWND != NULL);

    // We initiate the device, device context and swap chain.
    DXGI_SWAP_CHAIN_DESC scDesc;
    scDesc.BufferDesc.Width = mWinWidth; 		// Using the window's size avoids weird effects. If 0 the window's client width is used.
    scDesc.BufferDesc.Height = mWinHeight;		// Using the window's size avoids weird effects. If 0 the window's client height is used.
    scDesc.BufferDesc.RefreshRate.Numerator = 0;	// Screen refresh rate as RationalNumber. Zeroing it out makes DXGI calculate it.
    scDesc.BufferDesc.RefreshRate.Denominator = 0;	// Screen refresh rate as RationalNumber. Zeroing it out makes DXGI calculate it.
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// The most common format. Variations include [...]UNORM_SRGB.
    scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// The order pixel rows are drawn to the back buffer doesn't matter.
    scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					// Since the back buffer and window sizes matches, scaling doesn't matter.
    scDesc.SampleDesc.Count = 1;												// Disable multisampling.
    scDesc.SampleDesc.Quality = 0;												// Disable multisampling.
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						// The back buffer will be rendered to.
    scDesc.BufferCount = 1;							        // We only have one back buffer.
    scDesc.OutputWindow = mHWND;			                // Must point to the handle for the window used for rendering.
    scDesc.Windowed = true;					                // Run in windowed mode.
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	        // This makes the display driver select the most efficient technique.
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // Alt-enter fullscreen.

    DxAssert(D3D11CreateDeviceAndSwapChain(
        nullptr,					// Use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,	// Use the graphics card for rendering. Other options include software emulation.
        NULL,						// NULL since we don't use software emulation.
        D3D11_CREATE_DEVICE_DEBUG,	// Dbg creation flags.
        nullptr,					// Array of feature levels to try using. With null the following are used 11.0, 10.1, 10.0, 9.3, 9.2, 9.1.
        0,							// The array above has 0 elements.
        D3D11_SDK_VERSION,			// Always use this.
        &scDesc,					// Description of the swap chain.
        &mSwapChain,				// [out] The created swap chain.
        &mDevice,					// [out] The created device.
        nullptr,					// [out] The highest supported feature level (from array).
        &mDeviceContext				// [out] The created device context.
    ), S_OK);

    D3D11_VIEWPORT vp;
    vp.Width = (float)mWinWidth;
    vp.Height = (float)mWinHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    mDeviceContext->RSSetViewports(1, &vp);

    DxAssert(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&mBackBufferTex)), S_OK);
    DxAssert(mDevice->CreateRenderTargetView(mBackBufferTex, nullptr, &mBackBufferRTV), S_OK);

    // Clear render target.
    float clrColor[4] = { 0.f, 0.2f, 0.f, 0.f };
    mDeviceContext->ClearRenderTargetView(mBackBufferRTV, clrColor);
}

void Renderer::RenderCompanionWindow(ID3D11ShaderResourceView* leftEye, ID3D11ShaderResourceView* RightEye, ID3D11RenderTargetView* rtv) const
{
    mDeviceContext->OMSetRenderTargets(1, &rtv, nullptr);
    mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    mDeviceContext->VSSetShader(mScreenQuadVS, nullptr, 0);
    mDeviceContext->PSSetShader(mCompanionWindowPS, nullptr, 0);
    ID3D11ShaderResourceView* srv[] = { leftEye, RightEye };
    mDeviceContext->PSSetShaderResources(0, 2, srv);

    mDeviceContext->Draw(4, 0);

    void* p[1] = { NULL };
    mDeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)p, nullptr);
    mDeviceContext->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)p);
    mDeviceContext->PSSetShaderResources(1, 1, (ID3D11ShaderResourceView**)p);
    mDeviceContext->VSSetShader(NULL, nullptr, 0);
    mDeviceContext->PSSetShader(NULL, nullptr, 0);
}
