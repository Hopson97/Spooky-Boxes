#pragma once

#include <memory>
#include <vector>

#include <bullet/btBulletDynamicsCommon.h>

struct PhysicsObject
{
    std::unique_ptr<btCollisionShape> collision_shape;
    std::unique_ptr<btDefaultMotionState> motion_state;
    std::unique_ptr<btRigidBody> body;

    bool is_box = false;
};

class PhysicsSystem
{
  public:
    PhysicsSystem();

  public:
    btDiscreteDynamicsWorld world_;
    std::vector<PhysicsObject> objects_;

  private:
    btDefaultCollisionConfiguration config_;
    btCollisionDispatcher collision_dispatcher_;
    btSequentialImpulseConstraintSolver constraint_solver_;

    std::unique_ptr<btBroadphaseInterface> broad_phase_;
};