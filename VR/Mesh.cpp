#include "Mesh.hpp"
#include "DxHelp.hpp"

Mesh::Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Material* material)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mMaterial = material;
    mVertexBuffer = nullptr;
    mNumVertices = 0;
}

Mesh::~Mesh()
{
    if (mVertexBuffer != nullptr) mVertexBuffer->Release();
}

void Mesh::Load(std::vector<Material::Vertex>& vertexArr)
{
    assert(vertexArr.size() != 0);
    mNumVertices = (unsigned int)vertexArr.size();
    DxHelp::CreateVertexBuffer(mpDevice, mNumVertices, vertexArr.data(), &mVertexBuffer);
}
