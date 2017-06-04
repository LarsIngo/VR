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
        // particleCount Number of particles in system.
        ParticleEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int particleCount);

        // Destructor.
        ~ParticleEmitter();

    private:
        ID3D11Device* mpDevice;
        ID3D11DeviceContext* mpDeviceContext;

        StorageSwapBuffer* mPositionBuffer;
        StorageSwapBuffer* mOldPositionBuffer;
        StorageSwapBuffer* mVelocityBuffer;
        StorageSwapBuffer* mLifetimeBuffer;

        unsigned int mParticleCount;
};
