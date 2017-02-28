#include "AudioFile.hpp"
#include "AudioSystem.hpp"
#include <assert.h>

AudioFile::AudioFile()
{

}

AudioFile::~AudioFile()
{

}

void AudioFile::Load(SNDFILE* sndFile, SF_INFO& info, AudioSystem* audioSystem)
{
    mAudioSystem = audioSystem;
    mSndFile = sndFile;
    mInfo = info;
    mDuration = mInfo.frames / mInfo.samplerate;
}

void AudioFile::Play(bool loop, float phase, float volumeLeft, float volumeRight)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mPlay = true;
    mLoop = loop;
    mVolumeLeft = volumeLeft;
    mVolumeRight = volumeRight;
    mSfCount = mInfo.samplerate * phase;

    lock.unlock();
}

void AudioFile::Stop()
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mPlay = false;

    lock.unlock();
}

void AudioFile::End()
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = false;

    lock.unlock();
}

void AudioFile::SetVolumeLeft(float volume)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeLeft = volume;

    lock.unlock();
}

void AudioFile::SetVolumeRight(float volume)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mVolumeRight = volume;

    lock.unlock();
}

void AudioFile::SetPhase(float phase)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mSfCount = mInfo.samplerate * phase;

    lock.unlock();
}

void AudioFile::SetLoop(bool loop)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    std::unique_lock<std::mutex> lock(mAudioSystem->mMutex, std::defer_lock);
    lock.lock();

    mLoop = loop;

    lock.unlock();
}