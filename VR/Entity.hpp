#pragma once

#include <glm/glm.hpp>
#include <vector>

class Mesh;
class Texture2D;

class Entity
{
    public:
        // Constructor.
        Entity();

        // Destructor.
        ~Entity();

        glm::vec3 mPosition;

        // Mesh.
        Mesh* mpMesh;

        // Texture2D.
        Texture2D* mpTexture2D;
};
