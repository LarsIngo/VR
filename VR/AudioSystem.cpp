#include "AudioSystem.hpp"
#include <assert.h>
#include <iostream>
#include "FrameBuffer.hpp"

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
        FRAMES_PER_CHANNEL,
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

    for (auto& it : mAudioDataMap)
    {
        delete it.second.mAudioBuffer;
    }
    mAudioDataMap.clear();

    PaErrCheck(Pa_StopStream(mStream));
    PaErrCheck(Pa_CloseStream(mStream));

    PaErrCheck(Pa_Terminate());
}

AudioFile* AudioSystem::Load(const char* filePath)
{
    std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);

    lock.lock();

    if (mAudioDataMap.find(filePath) == mAudioDataMap.end())
    {
        AudioData& audioData = mAudioDataMap[filePath];
        SNDFILE* sndFile = sf_open(filePath, SFM_READ, &audioData.mAudioInfo);
        if (!sndFile)
            assert(0 && "sf_open");
        if (!sf_format_check(&audioData.mAudioInfo))
            assert(0 && "sf_format_check");
        assert(audioData.mAudioInfo.channels <= NUM_CHANNELS);

        audioData.mAudioBuffer = new float[audioData.mAudioInfo.frames * audioData.mAudioInfo.channels];
        sf_readf_float(sndFile, audioData.mAudioBuffer, audioData.mAudioInfo.frames);

        if (sf_close(sndFile))
            assert(0 && "sf_close");
    }
    
    assert(mNumAudioFiles < mMaxNumAudioFiles);
    AudioData& audioData = mAudioDataMap[filePath];
    AudioFile* audioFile = &mAudioFileArray[mNumAudioFiles++];
    audioFile->Load(&audioData, this);
    
    lock.unlock();

    return audioFile;
}

void AudioSystem::mUpdate()
{
    while (!mShutdown)
    {
        std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);
        lock.lock();

        std::memset(mBufferOut, SAMPLE_SILENCE, BUFFER_SIZE);
        unsigned int audioCount = 0;

        for (unsigned int audioFileIndex = 0; audioFileIndex < mNumAudioFiles; ++audioFileIndex)
        {
            AudioFile& audioFile = mAudioFileArray[audioFileIndex];
            if (audioFile.mPlay)
            {
                audioCount++;
                SF_INFO& info = audioFile.mpAudioData->mAudioInfo;
                sf_count_t frameStart = audioFile.mCurrFrame;
                sf_count_t frameEnd = std::min(audioFile.mCurrFrame + info.channels * FRAMES_PER_CHANNEL, info.frames);
                sf_count_t frameCount = frameEnd - frameStart;
                sf_count_t f = 0;
                float* bufferIn = audioFile.mpAudioData->mAudioBuffer;

                if (info.channels == 1)
                {   // MONO -> STEREO
                    for (sf_count_t frameWalker = frameStart; frameWalker < frameEnd; ++frameWalker)
                    {
                        // Left.
                        assert(f < FRAMES_PER_BUFFER);
                        mBufferOut[f++] += mMixAudio(frameWalker, bufferIn, info, audioFile) * audioFile.mVolumeLeft;
                        // Right.
                        assert(f < FRAMES_PER_BUFFER);
                        mBufferOut[f++] += mMixAudio(frameWalker, bufferIn, info, audioFile) * audioFile.mVolumeRight;
                    }
                }
                else
                {   // STEREO
                    for (sf_count_t frameWalker = frameStart; frameWalker < frameEnd; frameWalker += 2)
                    {
                        // Left.
                        assert(f < FRAMES_PER_BUFFER);
                        mBufferOut[f++] += mMixAudio(frameWalker, bufferIn, info, audioFile) * audioFile.mVolumeLeft;
                        // Right.
                        assert(f < FRAMES_PER_BUFFER);
                        mBufferOut[f++] += mMixAudio(frameWalker + 1, bufferIn, info, audioFile) * audioFile.mVolumeRight;
                    }
                }

                audioFile.mCurrFrame = frameEnd;
                if (audioFile.mpAudioData->mAudioInfo.frames == frameEnd)
                {
                    audioFile.mCurrFrame = 0;
                    audioFile.mPlay = audioFile.mLoop;
                }
            }
        }
        lock.unlock();

        if (audioCount > 0)
        {
            //for (int f = 0; f < readCount * sfInfo.channels; ++f)
            //    if (f % 2 == 0) sampleBlock[f] = 0.f;
            for (int f = 0; f < FRAMES_PER_BUFFER; ++f)
            {
                mBufferOut[f] = mBufferOut[f] / audioCount;
            }
            PaErrCheck(Pa_WriteStream(mStream, mBufferOut, FRAMES_PER_CHANNEL));
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

float AudioSystem::mMixAudio(sf_count_t frameWalker, float* buffer, SF_INFO& info, AudioFile& audioFile)
{
    float frameValue = buffer[frameWalker];
    unsigned int audioCount = 0;
    for (float delaySeconds : mAudioFilePhaseMap[&audioFile])
    {
        int delayFrames = (int)((float)delaySeconds * SAMPLE_RATE);
        int delayFrame = (int)frameWalker - delayFrames;

        if (delayFrame >= 0)
        {
            frameValue += buffer[delayFrame];
            ++audioCount;
        }
    }
    if (audioCount > 0)
        frameValue /= audioCount;

    return frameValue;
}

//float AudioSystem::mEchoFilter(unsigned int frameIndexIn, float* bufferIn, float* lastBufferIn)
//{
//    //http://stackoverflow.com/questions/5318989/reverb-algorithm
//    return 0.f;
//    //int delayMilliseconds = 500; // half a second
//    //int delaySamples =
//    //    (int)((float)delayMilliseconds * 44.1f); // assumes 44100 Hz sample rate
//    //float decay = 0.5f;
//    //for (int i = 0; i < buffer.length - delaySamples; i++)
//    //{
//    //    // WARNING: overflow potential
//    //    buffer[i + delaySamples] += (short)((float)buffer[i] * decay);
//    //}
//
//    //unsigned int delayFrames = FRAMES_PER_CHANNEL / 2;
//    //float decay = 0.5f;
//    //for (unsigned int f = 0; f < FRAMES_PER_BUFFER - delayFrames; ++f)
//    //{
//    //    bufferOut[f + delayFrames] += (short)((float)bufferIn[f] * decay);
//    //}
//    
//    //float frameValue = 0.f;
//    //for (unsigned int f = 0; f < BUFFER_NUM_FRAMES; ++f)
//    //{
//    //    frameValue += lastBufferIn[(BUFFER_NUM_FRAMES - 1) - f];
//    //}
//    //return frameValue / BUFFER_NUM_FRAMES;
//
//    //unsigned int frameDelay = 32;
//    //return (bufferIn[frameIndexIn] + lastBufferIn[(BUFFER_NUM_FRAMES - 1) - frameDelay]) / 2.f;
//
//    //for (unsigned int frameIT = 0; frameIT < numFrames; ++frameIT)
//    //{
//    //    float frameValue = 0.f;
//    //    unsigned int frameCount = 0;
//    //    for (unsigned int filterIT = 0; filterIT < filterSize; ++filterIT)
//    //    {
//    //        float scale = (float)filterIT / filterSize;
//    //        unsigned int frameIndex = frameIT + filterIT;
//    //        if (frameIndex < numFrames)
//    //        {
//    //            frameValue += buffer[frameIndex] * (1.f - scale);
//    //            ++frameCount;
//    //        }
//    //    }
//    //    frameValue /= frameCount;
//    //    buffer[frameIT] = frameValue;
//    //}
//}

void AudioSystem::UpdateAudioSource(Scene& scene, const glm::vec3& position, const glm::vec3& rightDirection, const glm::vec3& upDirection, glm::vec3& frontDirection)
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

        float volumeScale = 5.f;
        float volumeDistance = glm::clamp(1.f / audioDistance, 0.f, 1.f);
        float volumeFront = glm::clamp(glm::dot(frontDirection, audioVector), 0.f, 1.f);
        float volumeLeft = glm::clamp(glm::dot(-rightDirection, audioVector), 0.f, 1.f);
        float volumeRight = glm::clamp(glm::dot(rightDirection, audioVector), 0.f, 1.f);

        audioFile->mVolumeLeft = volumeScale * volumeDistance * (volumeFront + volumeLeft);
        audioFile->mVolumeRight = volumeScale * volumeDistance * (volumeFront + volumeRight);
    }
    lock.unlock();
}

void AudioSystem::UpdateAudioImage(Scene& scene, const glm::vec3& position, const glm::vec3& rightDirection, const glm::vec3& upDirection, glm::vec3& frontDirection, FrameBuffer* fb)
{
    std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);

    lock.lock();

    mAudioFilePhaseMap.clear();

    glm::vec4* worldArray = fb->ReadWorld();
    glm::vec4* normArray = fb->ReadNormal();

    unsigned int centerIndex = fb->mWidth * fb->mHeight / 2 + fb->mWidth / 2;
    glm::vec4 centerWorld = glm::vec4(worldArray[centerIndex]);
    glm::vec4 centerNormal = glm::vec4(normArray[centerIndex]);

    if (centerWorld.a > 0.5f && centerNormal.a > 0.5f)
    {
        glm::vec3 world = glm::vec3(centerWorld);
        glm::vec3 normal = glm::vec3(centerNormal);

        std::cout << "World: " << world.x << ", " << world.y << ", " << world.z << std::endl;
        std::cout << "Normal: " << normal.x << ", " << normal.y << ", " << normal.z << std::endl << std::endl;

        for (AudioSource& audioSource : scene.mAudioSourceList)
        {
            std::vector<float>& audioImagePhaseList = mAudioFilePhaseMap[audioSource.mpAudioFile];
            float phase = 0.5f;
            audioImagePhaseList.push_back(phase);
        }
    }

    lock.unlock();

    //std::unique_lock<std::mutex> lock(mMutex, std::defer_lock);
    
    //lock.lock();
    //for (AudioSource& audioSource : scene.mAudioSourceList)
    //{
    //    AudioFile* audioFile = audioSource.mpAudioFile;
    //    glm::vec3 audioVector = audioSource.mPosition - position;
    //    float audioDistance = glm::length(audioVector);
    //    if (audioDistance < 0.001f) audioVector += glm::vec3(0.f, 0.1f, 0.f);
    //    audioVector = glm::normalize(audioVector);

    //    float volumeScale = 5.f;
    //    float volumeDistance = glm::clamp(1.f / audioDistance, 0.f, 1.f);
    //    float volumeFront = glm::clamp(glm::dot(frontDirection, audioVector), 0.f, 1.f);
    //    float volumeLeft = glm::clamp(glm::dot(-rightDirection, audioVector), 0.f, 1.f);
    //    float volumeRight = glm::clamp(glm::dot(rightDirection, audioVector), 0.f, 1.f);

    //    audioFile->mVolumeLeft = volumeScale * volumeDistance * (volumeFront + volumeLeft);
    //    audioFile->mVolumeRight = volumeScale * volumeDistance * (volumeFront + volumeRight);
    //}
    //lock.unlock();
}
