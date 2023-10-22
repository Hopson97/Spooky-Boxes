#pragma once

#include "Mesh.h"
#include "OpenGL/Shader.h"
#include <bullet/btBulletDynamicsCommon.h>

struct PerspectiveCamera;

class DebugRenderer : public btIDebugDraw
{

  public:
    DebugRenderer(const PerspectiveCamera& camera);

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& from_colour,
                  const btVector3& to_colour) override;

    void drawLine(const btVector3& from, const btVector3& to,
                  const btVector3& colour) override;

    void drawSphere(const btVector3& p, btScalar radius, const btVector3& colour) override;

    void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c,
                      const btVector3& colour, btScalar alpha) override;

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normal_on_b,
                          btScalar distance, int lifeTime, const btVector3& colour) override;

    void reportErrorWarning(const char* warning_string) override;

    void draw3dText(const btVector3& location, const char* text) override;

    void setDebugMode(int debugMode) override;

    void render();

    int getDebugMode() const override;

  private:
    DebugMesh mesh_;
    Shader shader_;

    int debug_mode_ = 0;
    const PerspectiveCamera* p_camera_ = nullptr;
};