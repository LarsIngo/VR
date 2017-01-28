#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>

class FrameBuffer
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // width Width in pixels.
        // height Height in pixels.
        // bindFlags Bind flags.
        // initTexture Texture make frame buffer.
        FrameBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int width, unsigned int height, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, ID3D11Texture2D* initTexture = nullptr);

        // Destructor.
        ~FrameBuffer();

        // Clear color.
        void Clear(float r, float g, float b, float a);

        // Frame buffer width in pixels.
        unsigned int mWidth;
        // Frame buffer height in pixels.
        unsigned int mHeight;

        // Color.
        ID3D11Texture2D* mColTex;
        ID3D11ShaderResourceView* mColSRV;
        ID3D11RenderTargetView* mColRTV;
        ID3D11UnorderedAccessView* mColUAV;

        // Depth.
        ID3D11Texture2D* mDepthTex;
        ID3D11DepthStencilView* mDepthDSV;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
