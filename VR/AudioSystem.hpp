#pragma once

#include "AudioFile.hpp"

#include <sndfile.hh>
#include <windows.h>
#include <mmsystem.h>
#include <portaudio.h>
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <vector>

#define SAMPLE_RATE (44100/2)
#define PA_SAMPLE_TYPE (paFloat32)
#define SAMPLE_SIZE (4)
#define FRAMES_PER_BUFFER (512)
#define SAMPLE_SILENCE (0.0f)
#define NUM_SECONDS (10)
#define NUM_CHANNELS (2)
#define BUFFER_SIZE (FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE)

class AudioSystem
{
    friend AudioFile;
    public:
        // Constructor.
        AudioSystem();

        // Destructor.
        ~AudioSystem();

        // Load sound.
        AudioFile* Load(const char* filePath);

    private:
        std::thread* mThread;
        std::mutex mMutex;

        bool mShutdown = false;

        struct SFDATA
        {
            SNDFILE* sndFile;
            SF_INFO info;
        };
        std::map<std::string, SFDATA> mSFDataMap;
        std::vector<AudioFile> mAudioFileList;

        PaDeviceIndex mDeviceOut;
        const PaDeviceInfo* mDeviceInfoOut;
        PaStream* mStream = nullptr;
        PaStreamParameters mPaStreamOut;
        float mBufferIn[BUFFER_SIZE];
        float mBufferOut[BUFFER_SIZE];

        // Update audio system.
        void mUpdate();
};
