#include "Mesh.hpp"
#include "DxHelp.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

Assimp::Importer Mesh::aImporter = Assimp::Importer();

Mesh::Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Material* material)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mMaterial = material;
    mVertexBuffer = nullptr;
    mIndexBuffer = nullptr;
    mNumVertices = 0;
    mNumIndices = 0;
}

Mesh::~Mesh()
{
    if (mVertexBuffer != nullptr) mVertexBuffer->Release();
    if (mIndexBuffer != nullptr) mIndexBuffer->Release();
}

void Mesh::Load(const char* meshPath)
{
    assert(mVertexBuffer == nullptr && mIndexBuffer == nullptr && mNumVertices == 0 && mNumIndices == 0);

    const aiScene* aScene = aImporter.ReadFile(meshPath,
        aiProcess_CalcTangentSpace | \
        aiProcess_GenSmoothNormals | \
        aiProcess_JoinIdenticalVertices | \
        aiProcess_ImproveCacheLocality | \
        aiProcess_LimitBoneWeights | \
        aiProcess_RemoveRedundantMaterials | \
        aiProcess_SplitLargeMeshes | \
        aiProcess_Triangulate | \
        aiProcess_GenUVCoords | \
        aiProcess_SortByPType | \
        aiProcess_FindDegenerates | \
        aiProcess_FindInvalidData | \
        aiProcess_ValidateDataStructure | \
        0);

    std::cout << aImporter.GetErrorString() << std::endl;

    if (aScene != nullptr) LoadAssimpScene(aScene);

    //std::vector<Material::Vertex> vertexArr;
    //Material::Vertex vert;

    //vert.position = glm::vec3(0.f, 0.f, 1.f);
    //vert.normal = glm::vec3(0.f, 0.f, -1.f);
    //vert.uv = glm::vec2(0.f, 0.f);
    //vertexArr.push_back(vert);

    //vert.position = glm::vec3(0.5f, -0.5f, 1.f);
    //vert.normal = glm::vec3(0.f, 0.f, -1.f);
    //vert.uv = glm::vec2(1.f, 1.f);
    //vertexArr.push_back(vert);

    //vert.position = glm::vec3(0.f, -0.5f, 1.f);
    //vert.normal = glm::vec3(0.f, 0.f, -1.f);
    //vert.uv = glm::vec2(0.f, 1.f);
    //vertexArr.push_back(vert);

    //mNumVertices = (unsigned int)vertices.size();
    mNumVertices = (unsigned int)vertices.size();
    mNumIndices = (unsigned int)indices.size();
    DxHelp::CreateBuffer(mpDevice, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, &mVertexBuffer, vertices.data(), mNumVertices);
    DxHelp::CreateBuffer(mpDevice, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, &mIndexBuffer, indices.data(), mNumIndices);
}

void Mesh::LoadAssimpScene(const aiScene* aScene)
{
    assert(aScene != nullptr);

    std::vector<MeshEntry> entries;
    entries.resize(aScene->mNumMeshes);

    std::size_t numVertices = 0;
    std::size_t numIndices = 0;

    // Count the number of vertices and indices.
    for (unsigned int i = 0; i < aScene->mNumMeshes; ++i) {
        entries[i].numIndices = aScene->mMeshes[i]->mNumFaces * 3;
        entries[i].baseVertex = numVertices;
        entries[i].baseIndex = numIndices;

        numVertices += aScene->mMeshes[i]->mNumVertices;
        numIndices += entries[i].numIndices;
    }

    // Resize vectors to fit.
    vertices.resize(numVertices);
    //verticesPos.resize(numVertices);
    indices.resize(numIndices);

    numVertices = 0;
    numIndices = 0;

    // Initialize the meshes in the scene one by one.
    for (unsigned int m = 0; m < aScene->mNumMeshes; ++m) {
        const aiMesh* aMesh = aScene->mMeshes[m];
        // Load vertices.
        for (unsigned int i = 0; i < aMesh->mNumVertices; ++i) {
            Material::Vertex& vert = vertices[numVertices];
            CpyVec(vert.position, aMesh->mVertices[i]);
            CpyVec(vert.normal, aMesh->mNormals[i]);
            CpyVec(vert.uv, aMesh->mTextureCoords[0][i]);
            //CpyVec(vert.tangent, aMesh->mTangents[i]);
            //verticesPos[numVertices] = &vertices[numVertices].position;
            numVertices++;
        }
        // Load indices.
        for (unsigned int i = 0; i < aMesh->mNumFaces; ++i) {
            const aiFace& aFace = aMesh->mFaces[i];
            assert(aFace.mNumIndices == 3);
            indices[numIndices++] = entries[m].baseVertex + aFace.mIndices[0];
            indices[numIndices++] = entries[m].baseVertex + aFace.mIndices[1];
            indices[numIndices++] = entries[m].baseVertex + aFace.mIndices[2];
        }
    }
}

void Mesh::CpyVec(glm::vec3& glmVec, const aiVector3D& aiVec) {
    glmVec.x = aiVec.x;
    glmVec.y = aiVec.y;
    glmVec.z = aiVec.z;
}

void Mesh::CpyVec(glm::vec2& glmVec, const aiVector3D& aiVec) {
    glmVec.x = aiVec.x;
    glmVec.y = aiVec.y;
}


void Mesh::CpyVec(glm::vec2& glmVec, const aiVector2D& aiVec) {
    glmVec.x = aiVec.x;
    glmVec.y = aiVec.y;
}
