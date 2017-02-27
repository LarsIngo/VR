#include "AudioSystem.hpp"

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
        const std::string soundFile = "resources/assets/Audio/WinAssignment.wav"; //COHORT.WAV
        const int MAX_CHANNELS = 1;
        const size_t BUFFER_LEN = 4096;

        PaError paError = Pa_Initialize();
        if (paError != paNoError)
        {
            //message = str(format(playSoundStrings[error_pa_initialize_failed]) % Pa_GetErrorText(paError));
            //SoundWarning(message);
            //return false;
        }
        SNDFILE* sndFile = nullptr;
        SF_INFO sfInfo;
        ::memset(&sfInfo, 0, sizeof(sfInfo));
        sndFile = sf_open(soundFile.c_str(), SFM_READ, &sfInfo);
        if (!sndFile)
        {
            //message = str(format(playSoundStrings[error_sf_open_failed]) % soundFile.c_str() % sf_strerror(nullptr));
            //SoundWarning(message);
            //Pa_Terminate();
            //return false;
        }
        if (sfInfo.channels > MAX_CHANNELS)
        {
            //message = str(format(playSoundStrings[error_too_many_channels]) % sfInfo.channels % MAX_CHANNELS);
            //SoundWarning(message);
            //Pa_Terminate();
            //return false;
        }
        PaStream* stream = nullptr;
        PaStreamParameters paStreamParameters;
        paStreamParameters.device = Pa_GetDefaultOutputDevice();
        paStreamParameters.channelCount = sfInfo.channels;
        paStreamParameters.sampleFormat = paFloat32;
        paStreamParameters.suggestedLatency = Pa_GetDeviceInfo(paStreamParameters.device)->defaultLowOutputLatency;
        paStreamParameters.hostApiSpecificStreamInfo = nullptr;
        paError = Pa_OpenStream(
            &stream, nullptr, &paStreamParameters,
            sfInfo.samplerate, paFramesPerBufferUnspecified, paClipOff,
            nullptr, nullptr);
        if (paError != paNoError || !stream)
        {
            //message = str(format(playSoundStrings[error_pa_open_stream_failed]) % Pa_GetErrorText(paError));
            //SoundWarning(message);
            //Pa_Terminate();
            //return false;
        }
        paError = Pa_StartStream(stream);
        if (paError != paNoError)
        {
            //message = str(format(playSoundStrings[error_pa_start_stream_failed]) % Pa_GetErrorText(paError));
            //SoundWarning(message);
            //Pa_Terminate();
            //return false;
        }
        int subFormat = sfInfo.format & SF_FORMAT_SUBMASK;
        double scale = 1.0;
        if (subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE)
        {
            sf_command(sndFile, SFC_CALC_SIGNAL_MAX, &scale, sizeof(scale));
            if (scale < 1e-10)
            {
                scale = 1.0;
            }
            else
            {
                scale = 32700.0 / scale;
            }
        }
        sf_count_t readCount = 0;
        float data[BUFFER_LEN];
        ::memset(data, 0, sizeof(data));
        while ((readCount = sf_read_float(sndFile, data, BUFFER_LEN)))
        {
            if (subFormat == SF_FORMAT_FLOAT || subFormat == SF_FORMAT_DOUBLE)
            {
                int m = 0;
                for (m = 0; m < readCount; ++m)
                {
                    data[m] *= scale;
                }
            }
            paError = Pa_WriteStream(stream, data, BUFFER_LEN);
            if (paError != paNoError)
            {
                //message = str(format(playSoundStrings[error_pa_write_stream_failed]) % Pa_GetErrorText(paError));
                //SoundWarning(message);
                break;
            }
            ::memset(data, 0, sizeof(data));
        }
        paError = Pa_CloseStream(stream);
        if (paError != paNoError)
        {
            //message = str(format(playSoundStrings[error_pa_close_stream_failed]) % Pa_GetErrorText(paError));
            //SoundWarning(message);
            //Pa_Terminate();
            //return false;
        }
        Pa_Terminate();
        sf_close(sndFile);
    }
}
