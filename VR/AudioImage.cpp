#include "AudioImage.hpp"
#include "AudioFile.hpp"
#include "AudioSystem.hpp"

AudioImage::AudioImage(unsigned int width, unsigned int height, AudioSystem* audioSystem)
{
    mWidth = width;
    mHeight = height;
    mpAudioSystem = audioSystem;

    AudioFile* audioFile = mpAudioSystem->Load("");
}

AudioImage::~AudioImage()
{

}
