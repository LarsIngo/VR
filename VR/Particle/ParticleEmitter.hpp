#pragma once

#include <d3d11.h>

#include "../Camera.hpp"
#include "../Scene.hpp"

class StorageBuffer;
class StorageSwapBuffer;
class ParticleSystem;

class ParticleEmitter
{
    friend ParticleSystem;
    public:
        // Constructor.
        // pDevice Pointer to D3D11 device.
        // pDeviceContext Pointer to D3D11 device context.
        // emittFrequency Number of particles emitted per second.
        // lifetime Lifetime of particles in seconds.
        // velocity Velocity of particle.
        // scale Scale of particle.
        // color Color of particle.
        ParticleEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int emittFrequency, float lifeTime, const glm::vec3& velocity, const glm::vec2& scale, const glm::vec3& color);

        // Destructor.
        ~ParticleEmitter();

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;

        ID3D11ComputeShader* mEmittParticleCS;

        StorageSwapBuffer* mPositionBuffer;
        StorageSwapBuffer* mScaleBuffer;
        StorageSwapBuffer* mVelocityBuffer;
        StorageSwapBuffer* mLifetimeBuffer;
        StorageSwapBuffer* mColorBuffer;

        unsigned int mEmittFrequency;

        glm::vec3 mVelocity;
        glm::vec2 mScale;
        float mLifetime;
        glm::vec3 mColor;

        unsigned int mParticleCount;

        float mTime;
        unsigned int mEmittIndex;
};
