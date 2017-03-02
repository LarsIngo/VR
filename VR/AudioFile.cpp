#include "AudioFile.hpp"
#include "AudioSourceSystem.hpp"
#include <assert.h>

AudioFile::AudioFile()
{

}

AudioFile::~AudioFile()
{

}

void AudioFile::Load(AudioData* audioData, AudioSourceSystem* audioSourceSystem)
{
    mAudioSourceSystem = audioSourceSystem;
    mpAudioData = audioData;
    mDuration = mpAudioData->mAudioInfo.frames / mpAudioData->mAudioInfo.samplerate;
}

void AudioFile::Play(bool loop, float phase, float volumeLeft, float volumeRight)
{
    assert(mAudioSourceSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSourceSystem->mMutex, std::defer_lock);
    lock.lock();

    mPlay = true;
    mLoop = loop;
    mVolumeLeft = volumeLeft;
    mVolumeRight = volumeRight;
    mCurrFrame = mpAudioData->mAudioInfo.samplerate * phase;

    lock.unlock();
}

void AudioFile::Stop()
{
    assert(mAudioSourceSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSourceSystem->mMutex, std::defer_lock);
    lock.lock();

    mPlay = false;

    lock.unlock();
}

void AudioFile::End()
{
    assert(mAudioSourceSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSourceSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = false;

    lock.unlock();
}

void AudioFile::SetVolumeLeft(float volume)
{
    assert(mAudioSourceSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSourceSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeLeft = volume;

    lock.unlock();
}

void AudioFile::SetVolumeRight(float volume)
{
    assert(mAudioSourceSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSourceSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeRight = volume;

    lock.unlock();
}

void AudioFile::SetPhase(float phase)
{
    assert(mAudioSourceSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSourceSystem->mMutex, std::defer_lock);
    lock.lock();

    mCurrFrame = mpAudioData->mAudioInfo.samplerate * phase;

    lock.unlock();
}

void AudioFile::SetLoop(bool loop)
{
    assert(mAudioSourceSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSourceSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = loop;

    lock.unlock();
}