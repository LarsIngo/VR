#include "Entity.hpp"

Entity::Entity()
{
    mPosition = glm::vec3(0.f, 0.f, 0.f);
    mpMesh = nullptr;
    mpAlbedoTex = nullptr;
    mpNormalTex = nullptr;
	mpGlossTex = nullptr;
	mpMetalTex = nullptr;
}

Entity::~Entity()
{

}