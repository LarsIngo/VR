#include "ParticleEmitter.hpp"

#include "../StorageSwapBuffer.hpp"

ParticleEmitter::ParticleEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int emittFrequency, float lifeTime)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mEmittFrequency = emittFrequency;
    mLifetime = lifeTime;

    mParticleCount = mEmittFrequency * mLifetime;

    mTime = 0.f;
    mEmittIndex = 0.f;

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
