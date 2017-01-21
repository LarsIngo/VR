#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <string>

class Texture2D
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        Texture2D(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~Texture2D();

        // Load texture.
        // texPath Path to texture.
        void Load(const char* texPath);

        // Path to texture
        std::string mTexPath;
        // Texture width in pixels.
        unsigned int mWidth;
        // Texture height in pixels.
        unsigned int mHeight;
        // Texture format.
        DXGI_FORMAT mFormat;
        // Number of miplevels.
        unsigned int mMipLevels;

        // Texture.
        ID3D11Texture2D* mTex;
        // Texture view.
        ID3D11ShaderResourceView* mSRV;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
