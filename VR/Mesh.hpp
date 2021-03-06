#pragma once

#include <d3d11.h>

#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <vector>

class StorageBuffer;

class Mesh
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~Mesh();

        // Load mesh.
        // meshPath Path to mesh.
        void Load(const char* meshPath);

        // Number of vertices.
        unsigned int mNumVertices;

        //Number of indices.
        unsigned int mNumIndices;

        // Buffer.
        StorageBuffer* mPositionBuffer;
        StorageBuffer* mUVBuffer;
        StorageBuffer* mNormalBuffer;
        StorageBuffer* mTangentBuffer;

        // Index buffer.
        StorageBuffer* mIndexBuffer;

    private:
        struct MeshEntry {
            unsigned int numIndices = 0;
            unsigned int baseVertex = 0;
            unsigned int baseIndex = 0;
        };

        void LoadAssimpScene(const aiScene* aScene);
        void CpyVec(glm::vec3& glmVec, const aiVector3D& aiVec);
        void CpyVec(glm::vec2& glmVec, const aiVector3D& aiVec);
        void CpyVec(glm::vec2& glmVec, const aiVector2D& aiVec);

        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;

        static Assimp::Importer aImporter;
        std::vector<glm::vec3> positionList;
        std::vector<glm::vec2> uvList;
        std::vector<glm::vec3> normalList;
        std::vector<glm::vec3> tangentList;
        std::vector<unsigned int> indexList;
};
