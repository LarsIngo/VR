#pragma once

class FrameBuffer;

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
#include <vector>
#include "Scene.hpp"
#include <glm/glm.hpp>

#define PA_SAMPLE_TYPE (paFloat32)
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE (0)
#define NUM_CHANNELS (2)

#define FRAMES_PER_CHANNEL (1024)
#define SAMPLE_RATE (44100/2)
#define DURATION_PER_BUFFER ((float)FRAMES_PER_CHANNEL / SAMPLE_RATE)

#define FRAMES_PER_BUFFER (FRAMES_PER_CHANNEL * NUM_CHANNELS)
#define BUFFER_SIZE (FRAMES_PER_BUFFER * SAMPLE_SIZE)

#define AUDIO_VELOCITY (342.f / 8.f)
//#define DIRECT_AUDIO
#define ECHO_AUIDO
#define LINEAR_DEPTH_AUDIO

struct AudioData
{
    float* mAudioBuffer;
    SF_INFO mAudioInfo;
};

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

        // Update audio sources.
        void UpdateAudioSource(Scene& scene, const glm::vec3& position, const glm::vec3& rightDirection, const glm::vec3& upDirection, glm::vec3& frontDirection);

        // Update audio image.
        void UpdateAudioImage(Scene& scene, const glm::vec3& position, const glm::vec3& rightDirection, const glm::vec3& upDirection, glm::vec3& frontDirection, FrameBuffer* fb);

    private:
        std::thread mThread;
        std::mutex mMutex;

        bool mShutdown = false;

        std::map<std::string, AudioData> mAudioDataMap;

#ifdef ECHO_AUIDO
        struct AudioBounceData
        {
            float phase;
            float bounceFactor;
            float totalDistance;
        };
        std::map<AudioFile*, std::vector<AudioBounceData>> mAudioBounceDataMap;
#endif

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
        float mMixAudio(sf_count_t frameWalker, float* buffer, SF_INFO& info, AudioFile& audioFile, bool left);

        // Distance falloff function.
        float mDistanceFalloff(float distance);
};
