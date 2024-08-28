#ifndef COMPONENT_H
#define COMPONENT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <bitset>


using ComponentType = std::uint8_t;

const ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

struct Gravity
{
	glm::vec3 force;
};

struct RigidBody
{
	glm::vec3 velocity;
	glm::vec3 acceleration;
};

struct Transform
{
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
};

#endif // COMPONENT_H