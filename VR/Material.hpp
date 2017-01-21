#pragma once

#pragma comment(lib, "d3d11.lib")
#include <d3d11.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Material
{
    public:
        // Vertex
        struct Vertex
        {
            glm::vec3 position;
            glm::vec2 uv;
            glm::vec3 normal;
            glm::vec3 tangent;
        };

        struct GSMeta
        {
            glm::mat4 modelMatrix;
            glm::mat4 mvpMatrix;
        } mGSMeta;

        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        Material(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~Material();

        // Intialise.
        void Init(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, const char* VSPath, const char* GSPath, const char* PSPath);

        // D3D11 input layout.
        ID3D11InputLayout* mInputLayout;
        // Vertex shader.
        ID3D11VertexShader* mVS;
        // Geometry shader.
        ID3D11GeometryShader* mGS;
        // Pixel shader.
        ID3D11PixelShader* mPS;

        // GS Meta buffer.
        ID3D11ShaderResourceView* mGSMetaBuff;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
