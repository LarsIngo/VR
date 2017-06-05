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
        ParticleEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int emittFrequency, float lifeTime);

        // Destructor.
        ~ParticleEmitter();

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;

        ID3D11ComputeShader* mEmittParticleCS;

        StorageSwapBuffer* mPositionBuffer;
        StorageSwapBuffer* mOldPositionBuffer;
        StorageSwapBuffer* mVelocityBuffer;
        StorageSwapBuffer* mLifetimeBuffer;

        float mLifetime;
        unsigned int mEmittFrequency;

        unsigned int mParticleCount;

        float mTime;
        unsigned int mEmittIndex;
};
