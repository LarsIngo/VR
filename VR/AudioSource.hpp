#pragma once

class AudioFile;

#include <glm/glm.hpp>

class AudioSource
{
    public:
        // Constructor.
        AudioSource();

        // Destructor.
        ~AudioSource();

        // Position.
        glm::vec3 mPosition = glm::vec3(0.f, 0.f, 0.f);

        // Audio file.
        AudioFile* mpAudioFile = nullptr;

    private:

};

