#include "Entity.hpp"

Entity::Entity()
{
    mPosition = glm::vec3(0.f, 0.f, 0.f);
    mpMesh = nullptr;
}

Entity::~Entity()
{

}