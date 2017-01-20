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

void Mesh::Load(std::string meshPath)
{
    std::vector<Material::Vertex> vertexArr;
    Material::Vertex vert;

    vert.position = glm::vec3(0.f, 0.f, 1.f);
    vert.normal = glm::vec3(0.f, 0.f, -1.f);
    vert.uv = glm::vec2(0.f, 0.f);
    vertexArr.push_back(vert);

    vert.position = glm::vec3(0.5f, -0.5f, 1.f);
    vert.normal = glm::vec3(0.f, 0.f, -1.f);
    vert.uv = glm::vec2(1.f, 1.f);
    vertexArr.push_back(vert);

    vert.position = glm::vec3(0.f, -0.5f, 1.f);
    vert.normal = glm::vec3(0.f, 0.f, -1.f);
    vert.uv = glm::vec2(0.f, 1.f);
    vertexArr.push_back(vert);

    mNumVertices = (unsigned int)vertexArr.size();
    DxHelp::CreateVertexBuffer(mpDevice, mNumVertices, vertexArr.data(), &mVertexBuffer);
}
