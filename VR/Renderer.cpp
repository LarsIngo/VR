#include "Renderer.hpp"
#include "DxAssert.hpp"

#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer(unsigned int winWidth, unsigned int winHeight, bool fullscreen, unsigned int hmdRenderWidth, unsigned int hmdRenderHeight)
{
    mWinWidth = winWidth;
    mWinHeight = winHeight;
    mFullscreen = fullscreen;
    mHmdRenderWidth = hmdRenderWidth;
    mHmdRenderHeight = hmdRenderHeight;
    mClose = false;
    mHmdLeftTex = nullptr;
    mHmdRightTex = nullptr;
    mHmdLeftRTV = nullptr;
    mHmdRightRTV = nullptr;

    // Window
    InitialiseHWND();
    // DirectX
    InitialiseD3D();
    // VR
    if (mHmdRenderWidth != 0 && mHmdRenderHeight != 0)
        InitialiseHMD();
}

Renderer::~Renderer() 
{
    mDevice->Release();
    mDeviceContext->Release();
    mSwapChain->Release();
    mBackBufferRTV->Release();
    mBackBufferTex->Release();
    if (mHmdLeftTex != nullptr) mHmdLeftTex->Release();
    if (mHmdRightTex != nullptr) mHmdRightTex->Release();
    if (mHmdLeftRTV != nullptr) mHmdLeftRTV->Release();
    if (mHmdRightRTV != nullptr) mHmdRightRTV->Release();
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
    // Present to window.
    mSwapChain->Present(0, 0);
}

void Renderer::Render(Scene& scene, VRDevice& hmd) const
{
    vr::Texture_t leftEyeTexture = { mHmdLeftTex, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
    DxAssert(vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture), vr::VRCompositorError_None);
    vr::Texture_t rightEyeTexture = { mHmdRightTex, vr::TextureType_DirectX, vr::ColorSpace_Gamma };
    DxAssert(vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture), vr::VRCompositorError_None);

    // Copy left eye texture to back buffer.
    D3D11_BOX box;
    box.left = 0; box.top = 0; box.front = 0;
    box.right = mWinWidth; box.bottom = mWinHeight; box.back = 1;
    mDeviceContext->CopySubresourceRegion(mBackBufferTex, 0, 0, 0, 0, mHmdLeftTex, 0, &box);
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
    scDesc.BufferCount = 1;							// We only have one back buffer.
    scDesc.OutputWindow = mHWND;			// Must point to the handle for the window used for rendering.
    scDesc.Windowed = !mFullscreen;					// Run in windowed mode. Fullscreen is covered in a later sample.
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	// This makes the display driver select the most efficient technique.
    scDesc.Flags = 0;								// No additional options.

    DxAssert(D3D11CreateDeviceAndSwapChain(
        nullptr,					// Use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,	// Use the graphics card for rendering. Other options include software emulation.
        NULL,						// NULL since we don't use software emulation.
        D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_SINGLETHREADED,	// Dbg creation flags.
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

void Renderer::InitialiseHMD()
{
    assert(mHmdRenderWidth != 0 && mHmdRenderHeight != 0);

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Width = mHmdRenderWidth;
    texDesc.Height = mHmdRenderHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    DxAssert(mDevice->CreateTexture2D(&texDesc, NULL, &mHmdLeftTex), S_OK);;
    DxAssert(mDevice->CreateTexture2D(&texDesc, NULL, &mHmdRightTex), S_OK);;

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = texDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    DxAssert(mDevice->CreateRenderTargetView(mHmdLeftTex, &rtvDesc, &mHmdLeftRTV), S_OK);;
    DxAssert(mDevice->CreateRenderTargetView(mHmdRightTex, &rtvDesc, &mHmdRightRTV), S_OK);;

    // Clear render targets.
    {
        float clrColor[4] = { 0.2f, 0.f, 0.f, 0.f };
        mDeviceContext->ClearRenderTargetView(mHmdLeftRTV, clrColor);
    }
    {
        float clrColor[4] = { 0.f, 0.f, 0.2f, 0.f };
        mDeviceContext->ClearRenderTargetView(mHmdRightRTV, clrColor);
    }
}
