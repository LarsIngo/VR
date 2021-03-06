#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Scene.hpp"

class FrameBuffer;

class Material
{
    public:
        struct GSMeta
        {
            glm::mat4 modelMatrix;
            glm::mat4 mvpMatrix;
        } mGSMeta;

        struct PSMeta
        {
            glm::vec3 cameraPostion;
			unsigned int skyboxMipLevels;
        } mPSMeta;

        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        Material(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~Material();

        // Intialise.
        void Init(const char* VSPath, const char* GSPath, const char* PSPath);

        // Render scene.
        // scene Scene to render.
		// cameraPosition Position of the camera in world space.
        // vpMatrix Camera view projection matrix.
        // targetFb Frame buffer to render.
        void Render(Scene& scene, const glm::vec3& cameraPosition, const glm::mat4& vpMatrix, FrameBuffer* targetFb);

        // Vertex shader.
        ID3D11VertexShader* mVS;
        // Geometry shader.
        ID3D11GeometryShader* mGS;
        // Pixel shader.
        ID3D11PixelShader* mPS;
        // GS Meta buffer.
        ID3D11ShaderResourceView* mGSMetaBuff;
        // PS Meta buffer.
        ID3D11ShaderResourceView* mPSMetaBuff;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
