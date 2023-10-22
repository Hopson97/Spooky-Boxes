#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
    : collision_dispatcher_(&config_)
    , broad_phase_(std::make_unique<btDbvtBroadphase>())
    , world_(&collision_dispatcher_, broad_phase_.get(),
                                           &constraint_solver_, &config_)
{
    world_.setGravity({0, -10, 0});
}