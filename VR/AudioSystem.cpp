#include "AudioSystem.hpp"
#include <assert.h>
#include <iostream>

#define PaErrCheck(paError) if (paError != paNoError) { const char* strError = Pa_GetErrorText(paError); std::cout << strError << std::endl; assert(0 && strError); }

AudioSystem::AudioSystem()
{
    PaErrCheck(Pa_Initialize());
    //const PaDeviceIndex deviceIn = Pa_GetDefaultInputDevice();
    //const PaDeviceInfo* deviceInInfo = Pa_GetDeviceInfo(deviceIn);
    mDeviceOut = Pa_GetDefaultOutputDevice();
    mDeviceInfoOut = Pa_GetDeviceInfo(mDeviceOut);

    //PaStreamParameters paStreamIn;
    //paStreamIn.device = deviceIn;
    //paStreamIn.channelCount = sfInfo.channels;
    //paStreamIn.sampleFormat = PA_SAMPLE_TYPE;
    //paStreamIn.suggestedLatency = deviceInInfo->defaultLowInputLatency; // defaultHighInputLatency
    //paStreamIn.hostApiSpecificStreamInfo = NULL;

    mPaStreamOut.device = mDeviceOut;
    mPaStreamOut.channelCount = NUM_CHANNELS;
    mPaStreamOut.sampleFormat = PA_SAMPLE_TYPE;
    mPaStreamOut.suggestedLatency = mDeviceInfoOut->defaultLowOutputLatency; // defaultHighOutputLatency
    mPaStreamOut.hostApiSpecificStreamInfo = NULL;

    assert(NUM_CHANNELS <= mDeviceInfoOut->maxOutputChannels);

    PaErrCheck(Pa_OpenStream(
        &mStream,
        NULL,
        &mPaStreamOut,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        NULL,
        NULL));

    PaErrCheck(Pa_StartStream(mStream));

    mThread = std::thread(&AudioSystem::mUpdate, this);
}

AudioSystem::~AudioSystem()
{
    mShutdown = true;
    mThread.join();

    for (auto& it : mSFDataMap)
    {
        if (sf_close(it.second.sndFile))
            assert(0 && "sf_close");
    }
    mSFDataMap.clear();

    PaErrCheck(Pa_StopStream(mStream));
    PaErrCheck(Pa_CloseStream(mStream));

    PaErrCheck(Pa_Terminate());
}

AudioFile* AudioSystem::Load(const char* filePath)
{
    std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);

    lock.lock();

    if (mSFDataMap.find(filePath) == mSFDataMap.end())
    {
        SFDATA& sfData = mSFDataMap[filePath];
        sfData.sndFile = sf_open(filePath, SFM_READ, &sfData.info);
        if (!sfData.sndFile)
            assert(0 && "sf_open");
        if (!sf_format_check(&sfData.info))
            assert(0 && "sf_format_check");
        assert(sfData.info.channels <= NUM_CHANNELS);
    }
    
    assert(mNumAudioFiles < mMaxNumAudioFiles);
    SFDATA& sfData = mSFDataMap[filePath];
    AudioFile* audioFile = &mAudioFileArray[mNumAudioFiles++];
    audioFile->Load(sfData.sndFile, sfData.info, this);
    
    lock.unlock();

    return audioFile;
}

void AudioSystem::mUpdate()
{
    while (!mShutdown)
    {
        std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);
        
        std::memset(mBufferOut, SAMPLE_SILENCE, BUFFER_SIZE);
        unsigned int audioCount = 0;
        float maxAmplitude = 0.f;
        float amplitude;

        lock.lock();
        for (AudioFile& audioFile : mAudioFileArray)
        {
            if (audioFile.mPlay)
            {
                audioCount++;
                sf_seek(audioFile.mSndFile, audioFile.mSfCount, SEEK_SET);
                std::memset(mBufferIn, SAMPLE_SILENCE, BUFFER_SIZE);
                sf_count_t readCount = sf_readf_float(audioFile.mSndFile, mBufferIn, FRAMES_PER_BUFFER);
                if (audioFile.mInfo.channels == 1)
                {   // MONO -> STEREO
                    for (int f = 0; f < readCount * 2; ++f)
                    {
                        // Left.
                        amplitude = mBufferIn[f] * audioFile.mVolumeLeft;
                        maxAmplitude = std::fmaxf(amplitude, maxAmplitude);
                        mBufferOut[2 * f] += amplitude;
                        // Right.
                        amplitude = mBufferIn[f] * audioFile.mVolumeRight;
                        maxAmplitude = std::fmaxf(amplitude, maxAmplitude);
                        mBufferOut[2 * f + 1] += amplitude;
                    }
                }
                else
                {   // STEREO
                    for (int f = 0; f < readCount * 2; f += 2)
                    {
                        // Left.
                        amplitude = mBufferIn[f] * audioFile.mVolumeLeft;
                        maxAmplitude = std::fmaxf(amplitude, maxAmplitude);
                        mBufferOut[f] += amplitude;
                        // Right.
                        amplitude = mBufferIn[f + 1] * audioFile.mVolumeRight;
                        maxAmplitude = std::fmaxf(amplitude, maxAmplitude);
                        mBufferOut[f + 1] += amplitude;
                    }
                }

                audioFile.mSfCount += readCount;
                if (readCount == 0)
                {
                    audioFile.mSfCount = 0;
                    audioFile.mPlay = audioFile.mLoop;
                }
            }
        }
        lock.unlock();

        if (audioCount > 0)
        {
            //for (int f = 0; f < readCount * sfInfo.channels; ++f)
            //    if (f % 2 == 0) sampleBlock[f] = 0.f;
            for (int f = 0; f < FRAMES_PER_BUFFER * 2; ++f)
            {
                mBufferOut[f] = mBufferOut[f] / audioCount;
            }
            //Eco(mBufferOut, BUFFER_NUM_FRAMES);
            PaErrCheck(Pa_WriteStream(mStream, mBufferOut, FRAMES_PER_BUFFER));
        }
    }

    // +++ RECORD LIVE MIC +++ //
    //int numBytes = FRAMES_PER_BUFFER * sfInfo.channels * SAMPLE_SIZE;
    //char* sampleBlock = (char*)std::malloc(numBytes);
    //assert(sampleBlock != NULL);
    //std::memset(sampleBlock, SAMPLE_SILENCE, numBytes);

    //paError = Pa_StartStream(stream);
    //if (paError != paNoError)
    //{
    //    const char* err = Pa_GetErrorText(paError);
    //    assert(0 && err);
    //}

    //for (int i = 0; i< (NUM_SECONDS * SAMPLE_RATE) / FRAMES_PER_BUFFER; ++i)
    //{
    //    // You may get underruns or overruns if the output is not primed by PortAudio.
    //    paError = Pa_WriteStream(stream, sampleBlock, FRAMES_PER_BUFFER);
    //    if (paError != paNoError)
    //    {
    //        const char* err = Pa_GetErrorText(paError);
    //        assert(0 && err);
    //    }

    //    paError = Pa_ReadStream(stream, sampleBlock, FRAMES_PER_BUFFER);
    //    if (paError != paNoError)
    //    {
    //        const char* err = Pa_GetErrorText(paError);
    //        assert(0 && err);
    //    }
    //}
    //std::free(sampleBlock);
}

void AudioSystem::Eco(float* buffer, unsigned int numFrames)
{
    unsigned int filterSize = 64;
    for (unsigned int frameIT = 0; frameIT < numFrames; ++frameIT)
    {
        float frameValue = 0.f;
        unsigned int frameCount = 0;
        for (unsigned int filterIT = 0; filterIT < filterSize; ++filterIT)
        {
            float scale = (float)filterIT / filterSize;
            unsigned int frameIndex = frameIT + filterIT;
            if (frameIndex < numFrames)
            {
                frameValue += buffer[frameIndex] * (1.f - scale);
                ++frameCount;
            }
        }
        frameValue /= frameCount;
        buffer[frameIT] = frameValue;
    }
}

void AudioSystem::Update(Scene& scene, const glm::vec3& position, const glm::vec3& rightDirection, const glm::vec3& upDirection, glm::vec3& frontDirection)
{
    std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);

    lock.lock();
    for (AudioSource& audioSource : scene.mAudioSourceList)
    {
        AudioFile* audioFile = audioSource.mpAudioFile;
        glm::vec3 audioVector = audioSource.mPosition - position;
        float audioDistance = glm::length(audioVector);
        if (audioDistance < 0.001f) audioVector += glm::vec3(0.f, 0.1f, 0.f);
        audioVector = glm::normalize(audioVector);

        float volumeScale = 15.f;
        float volumeDistance = glm::clamp(1.f / audioDistance, 0.f, 1.f);
        float volumeFront = glm::clamp(glm::dot(frontDirection, audioVector), 0.f, 1.f);
        float volumeLeft = glm::clamp(glm::dot(-rightDirection, audioVector), 0.f, 1.f);
        float volumeRight = glm::clamp(glm::dot(rightDirection, audioVector), 0.f, 1.f);

        audioFile->mVolumeLeft = volumeScale * volumeDistance * (volumeFront + volumeLeft);
        audioFile->mVolumeRight = volumeScale * volumeDistance * (volumeFront + volumeRight);
    }
    lock.unlock();
}
