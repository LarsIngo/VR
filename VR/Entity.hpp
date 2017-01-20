#pragma once

#include <glm/glm.hpp>
#include <vector>

class Mesh;

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
};
