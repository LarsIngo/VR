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

#define SAMPLE_RATE (44100/2)
#define PA_SAMPLE_TYPE (paFloat32)
#define SAMPLE_SIZE (4)
#define FRAMES_PER_BUFFER (512)
#define SAMPLE_SILENCE (0.0f)
#define NUM_SECONDS (10)
#define NUM_CHANNELS (2)
#define BUFFER_NUM_FRAMES (FRAMES_PER_BUFFER * NUM_CHANNELS)
#define BUFFER_SIZE (BUFFER_NUM_FRAMES * SAMPLE_SIZE)

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

        void Update(Scene& scene, const glm::vec3& position, const glm::vec3& rightDirection, const glm::vec3& upDirection, glm::vec3& frontDirection);

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

        static const unsigned int mMaxNumAudioFiles = 50;
        unsigned int mNumAudioFiles = 0;
        std::array<AudioFile, mMaxNumAudioFiles> mAudioFileArray;

        PaDeviceIndex mDeviceOut;
        const PaDeviceInfo* mDeviceInfoOut;
        PaStream* mStream = nullptr;
        PaStreamParameters mPaStreamOut;
        float mBufferIn[BUFFER_NUM_FRAMES];
        float mBufferOut[BUFFER_NUM_FRAMES];

        // Update audio system.
        void mUpdate();

        // Eco audio effect.
        static void Eco(float* buffer, unsigned int numFrames);
};
