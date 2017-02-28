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

void AudioFile::Play(bool loop, float phase, float volume)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    mPlay = true;
    mLoop = loop;
    mVolume = volume;
    mSfCount = mInfo.samplerate * phase;
}

void AudioFile::Stop()
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    mPlay = false;
}

void AudioFile::End()
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    mLoop = false;
}

void AudioFile::SetVolume(float volume)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    mVolume = volume;
}

void AudioFile::SetPhase(float phase)
{
    assert(mSndFile != nullptr && mAudioSystem != nullptr);
    mSfCount = mInfo.samplerate * phase;
}

