#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>

class FrameBuffer
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // width Width in pixels.
        // height Height in pixels.
        // bindFlags Bind flags.
		// miscFlags Misc flags.
        // initTexture Texture make frame buffer.
        FrameBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int width, unsigned int height, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS, UINT miscFlags = 0, ID3D11Texture2D* initTexture = nullptr);

        // Destructor.
        ~FrameBuffer();

        // Clear textures.
        void ClearAll(float r = 0.f, float g = 0.f, float b = 0.f, float a = 0.f, float depth = 1.f);

		// Copy other frame buffer.
		void Copy(FrameBuffer* fb);

        // Read world texture.
        glm::vec4*  ReadWorld();
        // Read normal texture.
        glm::vec4*  ReadNormal();
        // Read depth texture.
        float*  ReadDepth();

        // Frame buffer width in pixels.
        unsigned int mWidth;
        // Frame buffer height in pixels.
        unsigned int mHeight;
		// Number of mip levels.
		unsigned int mMipLevels;

        // Color.
        ID3D11Texture2D* mColTex;
        ID3D11ShaderResourceView* mColSRV;
        ID3D11RenderTargetView* mColRTV;
        ID3D11UnorderedAccessView* mColUAV;

        // World.
        ID3D11Texture2D* mWorldTex;
        ID3D11ShaderResourceView* mWorldSRV;
        ID3D11RenderTargetView* mWorldRTV;
        ID3D11UnorderedAccessView* mWorldUAV;

        // Normal.
        ID3D11Texture2D* mNormTex;
        ID3D11ShaderResourceView* mNormSRV;
        ID3D11RenderTargetView* mNormRTV;
        ID3D11UnorderedAccessView* mNormUAV;

        // Depth.
        ID3D11Texture2D* mDepthTex;
        ID3D11ShaderResourceView* mDepthSRV;
        ID3D11RenderTargetView* mDepthRTV;
        ID3D11UnorderedAccessView* mDepthUAV;

        // Depth stencil.
        ID3D11Texture2D* mDepthStencilTex;
        ID3D11DepthStencilView* mDepthStencilDSV;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;

        // Staging textures.
        ID3D11Texture2D* mWorldStagingTex;
        ID3D11Texture2D* mNormStagingTex;
        ID3D11Texture2D* mDepthStagingTex;
};
