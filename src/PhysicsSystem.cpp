#include "PhysicsSystem.h"

PhysicsSystem::PhysicsSystem()
    : collision_dispatcher_(&config_)
    , world(&collision_dispatcher_, &broad_phase_, &constraint_solver_, &config_)
{
}

void PhysicsObject::setup(std::unique_ptr<btCollisionShape> collision_shape, float mass,
                          btVector3 position)
{
    this->collision_shape = std::move(collision_shape);
    btVector3 local_inertia(0, 0, 0);
    if (mass > 0.0f)
    {
        this->collision_shape->calculateLocalInertia(mass, local_inertia);
    }

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(position);

    motion_state = std::make_unique<btDefaultMotionState>(transform);
    btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state.get(),
                                                     this->collision_shape.get(), local_inertia);

    rb_info.m_friction = 0.9f;
    body = std::make_unique<btRigidBody>(rb_info);
    body->setUserPointer(this);
}