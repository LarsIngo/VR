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
#include <array>
#include "Scene.hpp"
#include <glm/glm.hpp>

#define PA_SAMPLE_TYPE (paFloat32)
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE (0.0f)
#define NUM_CHANNELS (2)

#define FRAMES_PER_CHANNEL (512)
#define SAMPLE_RATE (44100/2)
#define DURATION_PER_BUFFER ((float)FRAMES_PER_CHANNEL / SAMPLE_RATE)

#define FRAMES_PER_BUFFER (FRAMES_PER_CHANNEL * NUM_CHANNELS)
#define BUFFER_SIZE (FRAMES_PER_BUFFER * SAMPLE_SIZE)

struct AudioData
{
    float* mAudioBuffer;
    SF_INFO mAudioInfo;
};

class AudioSourceSystem
{
    friend AudioFile;
    public:
        // Constructor.
        AudioSourceSystem();

        // Destructor.
        ~AudioSourceSystem();

        // Load sound.
        AudioFile* Load(const char* filePath);

        void Update(Scene& scene, const glm::vec3& position, const glm::vec3& rightDirection, const glm::vec3& upDirection, glm::vec3& frontDirection);

    private:
        std::thread mThread;
        std::mutex mMutex;

        bool mShutdown = false;

        std::map<std::string, AudioData> mAudioDataMap;

        static const unsigned int mMaxNumAudioFiles = 50;
        unsigned int mNumAudioFiles = 0;
        AudioFile mAudioFileArray[mMaxNumAudioFiles];

        PaDeviceIndex mDeviceOut;
        const PaDeviceInfo* mDeviceInfoOut;
        PaStream* mStream = nullptr;
        PaStreamParameters mPaStreamOut;
        float mBufferOut[FRAMES_PER_BUFFER];

        // Update audio system.
        void mUpdate();

        // Mix buffers
        static float mMixAudio(sf_count_t frameWalker, float* bufferIn, SF_INFO& infoIn);

        // Echo audio effect.
        static float mEchoFilter(unsigned int frameIndexIn, float* bufferIn, float* lastBufferIn);
};
