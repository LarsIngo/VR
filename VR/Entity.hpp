#pragma once

#include <glm/glm.hpp>
#include <vector>

class Mesh;
class Texture2D;
class ParticleEmitter;

class Entity
{
    public:
        // Constructor.
        Entity();

        // Destructor.
        ~Entity();

        glm::vec3 mPosition;
		
		// Transparent.
		bool mTransparent;

        // Mesh.
        Mesh* mpMesh;

        // Albedo(Color).
        Texture2D* mpAlbedoTex;

        // Normal.
        Texture2D* mpNormalTex;

		// Gloss.
		Texture2D* mpGlossTex;

		// Metal.
		Texture2D* mpMetalTex;

        // Particle Emitter.
        ParticleEmitter* mpParticleEmitter;
};
