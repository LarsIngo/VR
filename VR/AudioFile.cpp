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
    mAudioSystem = audioSystem;
    mpAudioData = audioData;
    mDuration = mpAudioData->mAudioInfo.frames / mpAudioData->mAudioInfo.samplerate;
}

void AudioFile::Play(bool loop, float phase, float volumeLeft, float volumeRight)
{
    assert(mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
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
    assert(mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mPlay = false;

    lock.unlock();
}

void AudioFile::End()
{
    assert(mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = false;

    lock.unlock();
}

void AudioFile::SetVolumeLeft(float volume)
{
    assert(mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeLeft = volume;

    lock.unlock();
}

void AudioFile::SetVolumeRight(float volume)
{
    assert(mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeRight = volume;

    lock.unlock();
}

void AudioFile::SetPhase(float phase)
{
    assert(mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mCurrFrame = mpAudioData->mAudioInfo.samplerate * phase;

    lock.unlock();
}

void AudioFile::SetLoop(bool loop)
{
    assert(mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = loop;

    lock.unlock();
}