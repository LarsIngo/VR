#pragma once

#include <sndfile.hh>
#include <windows.h>
#include <mmsystem.h>
#include <portaudio.h>
#include <thread>

class AudioSystem
{
    public:
        // Constructor.
        AudioSystem();

        // Destructor.
        ~AudioSystem();

    private:
        std::thread* mThread;

        bool mShutdown = false;



        // Update audio system.
        void mUpdate();
};
