#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include "ecs.h"

class PhysicsSystem : public System
{
public:
	void Init();

	void Update(float dt);
};

#endif // PHYSICSSYSTEM_H