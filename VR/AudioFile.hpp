#pragma once

class AudioSystem;

#include <sndfile.hh>
#include <windows.h>
#include <mmsystem.h>
#include <portaudio.h>

class AudioFile
{
    friend AudioSystem;
    public:
        // Constructor.
        AudioFile();

        // Destructor.
        ~AudioFile();

        // Load sound.
        void Load(SNDFILE* sndFile, SF_INFO& info, AudioSystem* audioSystem);

        // Play sound.
        void Play(bool loop = false, float phase = 0, float volumeLeft = 1.f, float volumeRight = 1.f);

        // End sound.
        void End();

        // Set volume.
        void SetVolumeLeft(float volume);
        void SetVolumeRight(float volume);

        // Set phase.
        void SetPhase(float phase);

        // Set loop.
        void SetLoop(bool loop);

        // Stop sound.
        void Stop();

    private:
        SF_INFO mInfo;
        SNDFILE* mSndFile = nullptr;
        sf_count_t mSfCount = 0;
        bool mLoop = false;
        bool mPlay = false;
        float mVolumeLeft = 0.f;
        float mVolumeRight = 0.f;
        AudioSystem* mAudioSystem = nullptr;
        float mDuration = 0.f;
};

