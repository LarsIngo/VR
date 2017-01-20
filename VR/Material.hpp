#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>
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
        glm::vec3 normal;
        glm::vec2 uv;
    };

    // Constructor.
    // pDevice Pointer to D3D11 device.
    // pDeviceContext Pointer to D3D11 device context.
    Material(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

    // Destructor.
    ~Material();

    // Intialise.
    void Init(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputDesc, std::wstring& VSPath, std::wstring& GSPath, std::wstring& PSPath);

    // D3D11 input layout.
    ID3D11InputLayout* mInputLayout;
    // Vertex shader.
    ID3D11VertexShader* mVS;
    // Geometry shader.
    ID3D11GeometryShader* mGS;
    // Pixel shader.
    ID3D11PixelShader* mPS;

private:
    ID3D11Device* mpDevice;
    ID3D11DeviceContext* mpDeviceContext;
};
