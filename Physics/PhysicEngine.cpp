#include "PhysicEngine.h"

std::unique_ptr<PhysicsEngine::_ImplPhysic> PhysicsEngine::implementation{};
bool PhysicsEngine::isRunning = true;
