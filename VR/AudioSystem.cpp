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

    mThread = new std::thread(&AudioSystem::mUpdate, this);
}

AudioSystem::~AudioSystem()
{
    mShutdown = true;
    mThread->join();
    delete mThread;

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
    
    SFDATA& sfData = mSFDataMap[filePath];
    mAudioFileList.resize(mAudioFileList.size() + 1);
    AudioFile* audioFile = &mAudioFileList[mAudioFileList.size() - 1];
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

        lock.lock();
        for (AudioFile& audioFile : mAudioFileList)
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
                        mBufferOut[2 * f] += mBufferIn[f] * audioFile.mVolume;
                        mBufferOut[2 * f + 1] += mBufferIn[f] * audioFile.mVolume;
                    }
                }
                else
                {   // STEREO
                    for (int f = 0; f < readCount * 2; ++f)
                        mBufferOut[f] += mBufferIn[f] * audioFile.mVolume;
                }

                audioFile.mSfCount += readCount;
                if (readCount == 0 && audioFile.mLoop)
                {
                    audioFile.mSfCount = 0;
                }
            }
        }
        lock.unlock();

        if (audioCount > 0)
        {
            //for (int f = 0; f < readCount * sfInfo.channels; ++f)
            //    if (f % 2 == 0) sampleBlock[f] = 0.f;
            for (int f = 0; f < FRAMES_PER_BUFFER * 2; ++f)
                mBufferOut[f] = mBufferOut[f] / audioCount;
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
