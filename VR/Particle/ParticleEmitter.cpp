#include "ParticleEmitter.hpp"

#include "../StorageSwapBuffer.hpp"

ParticleEmitter::ParticleEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int particleCount)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mParticleCount = particleCount;

    mPositionBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mParticleCount, sizeof(glm::vec3));
    mOldPositionBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mParticleCount, sizeof(glm::vec3));
    mVelocityBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mParticleCount, sizeof(glm::vec3));
    mLifetimeBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(float) * mParticleCount, sizeof(float));

    std::vector<float> vec;
    vec.resize(mParticleCount);
    std::fill(vec.begin(), vec.end(), -1);
    mLifetimeBuffer->WriteAll(vec.data(), sizeof(float) * mParticleCount, 0);
}

ParticleEmitter::~ParticleEmitter()
{
    delete mPositionBuffer;
    delete mOldPositionBuffer;
    delete mVelocityBuffer;
    delete mLifetimeBuffer;
}
