#pragma once

class AudioFile;
class AudioSystem;
#include <glm/glm.hpp>
#include <vector>

class AudioImage
{
    public:
        // Constructor.
        AudioImage(unsigned int width, unsigned int height, AudioSystem* audioSystem);

        // Destructor.
        ~AudioImage();

        // Position.
        glm::vec3 mPosition = glm::vec3(0.f, 0.f, 0.f);

    private:
        std::vector<AudioFile*> mAudioFileList;
        AudioSystem* mpAudioSystem;
        unsigned int mWidth;
        unsigned int mHeight;
};

