#pragma once

#include <d3d11.h>
#include <random>

#include "../Camera.hpp"
#include "../Scene.hpp"

class StorageBuffer;

class ParticleSystem
{
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // particleCount Number of particles in system.
        ParticleSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);

        // Destructor.
        ~ParticleSystem();

        // Update particles.
        // scene Scene to update.
        // dt Delta time.
        void Update(Scene& scene, float dt);

        // Render particles.
        // scene Scene to render.
        // camera Camera to render from.
        void Render(Scene& scene, Camera& camera);

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;

        ID3D11ComputeShader* mEmittCS;
        ID3D11ComputeShader* mUpdateCS;

        ID3D11VertexShader* mVertexShader;
        ID3D11GeometryShader* mGeometryShader;
        ID3D11PixelShader* mPixelShader;
        ID3D11BlendState* mBlendState;

        struct EmittMetaData
        {
            glm::vec3 position;
            glm::vec3 velocity;
            float lifetime;
            unsigned int emittIndex;
            int emittPointIndex;
        } mEmittMetaData;
        StorageBuffer* mEmittMetaDataBuffer;

        struct UpdateMetaData
        {
            float dt;
            unsigned int particleCount;
            float pad[2];
        } mUpdateMetaData;
        StorageBuffer* mUpdateMetaDataBuffer;

        struct RenderMetaData
        {
            glm::mat4 vpMatrix;
            glm::vec3 lensPosition;
            glm::vec3 lensUpDirection;
            float pad[2];
        } mRenderMetaData;
        StorageBuffer* mRenderMetaDataBuffer;

        std::default_random_engine mRandomEngine;
};
