#include "Mesh.hpp"
#include "DxHelp.hpp"
#include "StorageBuffer.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

Assimp::Importer Mesh::aImporter = Assimp::Importer();

Mesh::Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;
    mPositionBuffer = nullptr;
    mUVBuffer = nullptr;
    mNormalBuffer = nullptr;
    mTangentBuffer = nullptr;
    mIndexBuffer = nullptr;
    mNumVertices = 0;
    mNumIndices = 0;
}

Mesh::~Mesh()
{
    if (mPositionBuffer != nullptr) delete mPositionBuffer;
    if (mUVBuffer != nullptr) delete mUVBuffer;
    if (mNormalBuffer != nullptr) delete mNormalBuffer;
    if (mTangentBuffer != nullptr) delete mTangentBuffer;
    if (mIndexBuffer != nullptr) delete mIndexBuffer;
}

void Mesh::Load(const char* meshPath)
{
    assert(mNumVertices == 0 && mNumIndices == 0);

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

    mNumVertices = (unsigned int)positionList.size();
    mNumIndices = (unsigned int)indexList.size();
    
    mPositionBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mNumVertices, sizeof(glm::vec3));
    mUVBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec2) * mNumVertices, sizeof(glm::vec2));
    mNormalBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mNumVertices, sizeof(glm::vec3));
    mTangentBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mNumVertices, sizeof(glm::vec3));
    mIndexBuffer = new StorageBuffer(mpDevice, mpDeviceContext, sizeof(unsigned int) * mNumIndices, sizeof(unsigned int));

    mPositionBuffer->Write(positionList.data(), sizeof(glm::vec3) * mNumVertices, 0);
    mUVBuffer->Write(uvList.data(), sizeof(glm::vec2) * mNumVertices, 0);
    mNormalBuffer->Write(normalList.data(), sizeof(glm::vec3) * mNumVertices, 0);
    mTangentBuffer->Write(tangentList.data(), sizeof(glm::vec3) * mNumVertices, 0);
    mIndexBuffer->Write(indexList.data(), sizeof(unsigned int) * mNumIndices, 0);

    positionList.clear();
    positionList.shrink_to_fit();
    uvList.clear();
    uvList.shrink_to_fit();
    normalList.clear();
    normalList.shrink_to_fit();
    tangentList.clear();
    tangentList.shrink_to_fit();
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
    positionList.resize(numVertices);
    uvList.resize(numVertices);
    normalList.resize(numVertices);
    tangentList.resize(numVertices);
    //verticesPos.resize(numVertices);
    indexList.resize(numIndices);

    numVertices = 0;
    numIndices = 0;

    // Initialize the meshes in the scene one by one.
    for (unsigned int m = 0; m < aScene->mNumMeshes; ++m) {
        const aiMesh* aMesh = aScene->mMeshes[m];
        // Load vertices.
        for (unsigned int i = 0; i < aMesh->mNumVertices; ++i) {
            CpyVec(positionList[numVertices], aMesh->mVertices[i]);
            CpyVec(uvList[numVertices], aMesh->mTextureCoords[0][i]);
            CpyVec(normalList[numVertices], aMesh->mNormals[i]);
            CpyVec(tangentList[numVertices], aMesh->mTangents[i]);
            numVertices++;
        }
        // Load indices.
        for (unsigned int i = 0; i < aMesh->mNumFaces; ++i) {
            const aiFace& aFace = aMesh->mFaces[i];
            assert(aFace.mNumIndices == 3);
            indexList[numIndices++] = entries[m].baseVertex + aFace.mIndices[0];
            indexList[numIndices++] = entries[m].baseVertex + aFace.mIndices[1];
            indexList[numIndices++] = entries[m].baseVertex + aFace.mIndices[2];
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
