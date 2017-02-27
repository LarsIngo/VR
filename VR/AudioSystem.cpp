#include "AudioSystem.hpp"
#include <assert.h>
#include <iostream>

#define SAMPLE_RATE 44100
#define PA_SAMPLE_TYPE paFloat32
#define SAMPLE_SIZE 4
#define FRAMES_PER_BUFFER 512
#define SAMPLE_SILENCE 0.0f
#define NUM_SECONDS 10

#define PaErrCheck(paError) if (paError != paNoError) { const char* strError = Pa_GetErrorText(paError); std::cout << strError << std::endl; assert(0 && strError); }

//typedef float SAMPLE;
//
//float CubicAmplifier(float input)
//{
//    float output, temp;
//    if (input < 0.0)
//    {
//        temp = input + 1.0f;
//        output = (temp * temp * temp) - 1.0f;
//    }
//    else 
//    {
//        temp = input - 1.0f;
//        output = (temp * temp * temp) + 1.0f;
//    }
//    return output;
//}
//
//#define FUZZ(x) CubicAmplifier(CubicAmplifier(CubicAmplifier(CubicAmplifier(x))))
//
//static int gNumNoInputs = 0;
//
//static int fuzzCallback(const void *inputBuffer,
//    void *outputBuffer,
//    unsigned long framesPerBuffer,
//    const PaStreamCallbackTimeInfo* timeInfo,
//    PaStreamCallbackFlags statusFlags,
//    void *userData)
//    {
//    SAMPLE *out = (SAMPLE*)outputBuffer;
//    const SAMPLE *in = (const SAMPLE*)inputBuffer;
//    unsigned int i;
//    
//     if (inputBuffer == NULL) 
//     {
//     for (i = 0; i<framesPerBuffer; i++)
//         {
//            *out++ = 0;   /* left  - silent */
//            *out++ = 0;   /* right - silent */
//         }
//        gNumNoInputs += 1;
//    }
//    else 
//    {
//        for (i = 0; i<framesPerBuffer; i++)
//        {
//                * out++ = FUZZ(*in++);   /* left  - distorted */
//                * out++ = *in++;         /* right - clean */
//        }
//    }
//    return paContinue;
//}

AudioSystem::AudioSystem()
{
    mThread = new std::thread(&AudioSystem::mUpdate, this);
}

AudioSystem::~AudioSystem()
{
    mShutdown = true;
    mThread->join();
    delete mThread;
}

void AudioSystem::mUpdate()
{
    while (!mShutdown)
    {
        const std::string soundFile = "resources/assets/Audio/COHORT.WAV"; //COHORT.WAV // WinAssignment

        PaErrCheck(Pa_Initialize());
        SF_INFO sfInfo; std::memset(&sfInfo, 0, sizeof(sfInfo));

        SNDFILE* sndFile = nullptr;
        sndFile = sf_open(soundFile.c_str(), SFM_READ, &sfInfo);
        if (!sf_format_check(&sfInfo))
            assert(0 && "sf_format_check");
        if (!sndFile)
            assert(0 && "sf_open");

        const PaDeviceIndex deviceIn = Pa_GetDefaultInputDevice();
        const PaDeviceIndex deviceOut = Pa_GetDefaultOutputDevice();
        const PaDeviceInfo* deviceInInfo = Pa_GetDeviceInfo(deviceIn);
        const PaDeviceInfo* deviceOutInfo = Pa_GetDeviceInfo(deviceOut);
        PaStream* stream = nullptr;

        //PaStreamParameters paStreamIn;
        //paStreamIn.device = deviceIn;
        //paStreamIn.channelCount = sfInfo.channels;
        //paStreamIn.sampleFormat = PA_SAMPLE_TYPE;
        //paStreamIn.suggestedLatency = deviceInInfo->defaultLowInputLatency; // defaultHighInputLatency
        //paStreamIn.hostApiSpecificStreamInfo = NULL;

        PaStreamParameters paStreamOut;
        paStreamOut.device = deviceOut;
        paStreamOut.channelCount = sfInfo.channels;
        paStreamOut.sampleFormat = PA_SAMPLE_TYPE;
        paStreamOut.suggestedLatency = deviceOutInfo->defaultLowOutputLatency; // defaultHighOutputLatency
        paStreamOut.hostApiSpecificStreamInfo = NULL;

        assert(sfInfo.channels <= deviceOutInfo->maxOutputChannels);

        PaErrCheck(Pa_OpenStream(
            &stream,
            NULL,
            &paStreamOut,
            sfInfo.samplerate,
            FRAMES_PER_BUFFER,
            paClipOff,
            NULL,
            NULL));

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

        int numBytes = FRAMES_PER_BUFFER * sfInfo.channels * SAMPLE_SIZE;
        float* sampleBlock = (float*)std::malloc(numBytes);
        assert(sampleBlock != NULL);
        std::memset(sampleBlock, SAMPLE_SILENCE, numBytes);

        PaErrCheck(Pa_StartStream(stream));
        sf_count_t readCount = 0;
        while (readCount = sf_read_float(sndFile, sampleBlock, FRAMES_PER_BUFFER))
        {
            PaErrCheck(Pa_WriteStream(stream, sampleBlock, FRAMES_PER_BUFFER));
            std::memset(sampleBlock, SAMPLE_SILENCE, numBytes);
        }
        
        PaErrCheck(Pa_StopStream(stream));
        std::free(sampleBlock);

        PaErrCheck(Pa_CloseStream(stream));
        PaErrCheck(Pa_Terminate());
        sf_close(sndFile);
    }
}
