#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <d3d11.h>
#include <d3dcompiler.inl>
#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Material.hpp"

class Mesh
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // material Mesh material.
        Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Material* material);

        // Destructor.
        ~Mesh();

        // Load mesh.
        // meshPath Path to mesh.
        void Load(std::string meshPath);

        // Material.
        Material* mMaterial;

        // Number of vertices.
        unsigned int mNumVertices;

        // Vertex buffer.
        ID3D11Buffer* mVertexBuffer;

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;
};
