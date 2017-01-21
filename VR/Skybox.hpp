#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <glm/glm.hpp>

class Texture2D;
class FrameBuffer;

class Skybox
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        Skybox(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~Skybox();

        void Load(
        // Back texture.
        Texture2D* mpBK,
        // Down texture.
        Texture2D* mpDN,
        // Front texture.
        Texture2D* mpFR,
        // Left texture.
        Texture2D* mpLF,
        // Right texture.
        Texture2D* mpRT,
        // Up texture.
        Texture2D* mpUP
        );

        // Render skybox to frame buffer.
        // projectionMatrix Camera projection matrix.
        // viewMatrix Camera view matrix.
        void Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, FrameBuffer* targetFb);

        // Cube map.
        ID3D11ShaderResourceView* mCubeMapSRV;

        // PS Meta buffer.
        struct PSMeta
        {
            glm::mat4 projectionMatrix;
            glm::mat4 viewMatrix;
        } mPSMeta;
        ID3D11ShaderResourceView* mPSMetaBuff;

        // Vertex shader.
        ID3D11VertexShader* mScreenQuadVS;
        // Pixel shader.
        ID3D11PixelShader* mSkyboxPS;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
