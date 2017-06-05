#include "ParticleEmitter.hpp"

#include "../StorageSwapBuffer.hpp"

ParticleEmitter::ParticleEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, unsigned int emittFrequency, float lifeTime, const glm::vec3& velocity, const glm::vec2& scale, const glm::vec3& color)
{
    mpDevice = pDevice;
    mpDeviceContext = pDeviceContext;

    mEmittFrequency = emittFrequency;
    mLifetime = lifeTime;
    mVelocity = velocity;
    mScale = scale;
    mColor = color;

    mParticleCount = mEmittFrequency * mLifetime;

    mTime = 0.f;
    mEmittIndex = 0;

    mPositionBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mParticleCount, sizeof(glm::vec3));
    mScaleBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec2) * mParticleCount, sizeof(glm::vec2));
    mVelocityBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mParticleCount, sizeof(glm::vec3));
    mLifetimeBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(float) * mParticleCount, sizeof(float));
    mColorBuffer = new StorageSwapBuffer(mpDevice, mpDeviceContext, sizeof(glm::vec3) * mParticleCount, sizeof(glm::vec3));

    std::vector<float> vec;
    vec.resize(mParticleCount);
    std::fill(vec.begin(), vec.end(), -1);
    mLifetimeBuffer->WriteAll(vec.data(), sizeof(float) * mParticleCount, 0);
}

ParticleEmitter::~ParticleEmitter()
{
    delete mPositionBuffer;
    delete mScaleBuffer;
    delete mVelocityBuffer;
    delete mLifetimeBuffer;
    delete mColorBuffer;
}
