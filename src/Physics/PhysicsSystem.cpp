#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
    : collision_dispatcher_(&config_)
    , world_(&collision_dispatcher_, &broad_phase_,
                                           &constraint_solver_, &config_)
{
    world_.setGravity({0, -10, 0});
}