#include "AudioFile.hpp"
#include "AudioSystem.hpp"
#include <assert.h>

AudioFile::AudioFile()
{

}

AudioFile::~AudioFile()
{

}

void AudioFile::Load(AudioData* audioData, AudioSystem* audioSystem)
{
    mpAudioSystem = audioSystem;
    mpAudioData = audioData;
    mDuration = (float)mpAudioData->mAudioInfo.frames / mpAudioData->mAudioInfo.samplerate;
}

void AudioFile::Play(bool loop, float phase, float volumeLeft, float volumeRight)
{
    assert(mpAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mpAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mPlay = true;
    mLoop = loop;
    mVolumeLeft = volumeLeft;
    mVolumeRight = volumeRight;
    assert(phase >= 0.f);
    mCurrFrame = (sf_count_t)(mpAudioData->mAudioInfo.samplerate * phase);

    lock.unlock();
}

void AudioFile::Stop()
{
    assert(mpAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mpAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mPlay = false;

    lock.unlock();
}

void AudioFile::End()
{
    assert(mpAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mpAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = false;

    lock.unlock();
}

void AudioFile::SetVolumeLeft(float volume)
{
    assert(mpAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mpAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeLeft = volume;

    lock.unlock();
}

void AudioFile::SetVolumeRight(float volume)
{
    assert(mpAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mpAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeRight = volume;

    lock.unlock();
}

void AudioFile::SetPhase(float phase)
{
    assert(mpAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mpAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    assert(phase >= 0.f);
    mCurrFrame = (sf_count_t)(mpAudioData->mAudioInfo.samplerate * phase);

    lock.unlock();
}

void AudioFile::SetLoop(bool loop)
{
    assert(mpAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mpAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = loop;

    lock.unlock();
}