#pragma once

#include <memory>
#include <vector>

#include <bullet/btBulletDynamicsCommon.h>

struct PhysicsObject
{
    std::unique_ptr<btCollisionShape> collision_shape;
    std::unique_ptr<btDefaultMotionState> motion_state;
    std::unique_ptr<btRigidBody> body;
    void setup(std::unique_ptr<btCollisionShape> collision_shape, float mass, btVector3 position);
};

class PhysicsSystem
{
  public:
    PhysicsSystem();

  public:
    btDiscreteDynamicsWorld world;
    std::vector<PhysicsObject> objects;

  private:
    btDefaultCollisionConfiguration config_;
    btCollisionDispatcher collision_dispatcher_;
    btSequentialImpulseConstraintSolver constraint_solver_;

    btDbvtBroadphase broad_phase_;
};