#include "DebugRenderer.h"

#include <glad/glad.h>
#include <iostream>

#include "Camera.h"
#include "OpenGL/VertexArray.h"

// Inspired from
// https://gamedev.stackexchange.com/questions/172789/bullet-physics-debugdraw-unexpected-results
// and
// https://github.com/Chaosed0/SpiderGame/blob/master/Engine/src/Renderer/BulletDebugDrawer.cpp

DebugRenderer::DebugRenderer(const PerspectiveCamera& camera)
    : p_camera_(&camera)
{
    if (!shader_.load_from_file("assets/shaders/DebugVertex.glsl",
                                "assets/shaders/DebugFragment.glsl"))
    {
        return;
    }
}

void DebugRenderer::drawLine(const btVector3& from, const btVector3& to,
                             const btVector3& from_colour, const btVector3& to_colour)
{
    DebugVertex from_vertex = {{from.x(), from.y(), from.z()},
                               {from_colour.x(), from_colour.y(), from_colour.z()}};
    DebugVertex to_vertex = {{to.x(), to.y(), to.z()},
                             {to_colour.x(), to_colour.y(), to_colour.z()}};

    mesh_.vertices.push_back(from_vertex);
    mesh_.vertices.push_back(to_vertex);

    mesh_.indices.push_back(static_cast<GLuint>(mesh_.indices.size()));
    mesh_.indices.push_back(static_cast<GLuint>(mesh_.indices.size()));

    // std::printf("Drawing a line from %f %f %f to %f %f %f colour %f %f %f\n", from[0],
    // from[1],
    //             from[2], to[0], to[1], to[2], from_colour[0], from_colour[1],
    //             from_colour[2]);
}

void DebugRenderer::drawLine(const btVector3& from, const btVector3& to,
                             const btVector3& colour)
{
    drawLine(from, to, colour, colour);
}

void DebugRenderer::drawSphere(const btVector3& p, btScalar radius, const btVector3& colour)
{
}

void DebugRenderer::drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c,
                                 const btVector3& colour, btScalar alpha)
{
    drawLine(a, b, colour, colour);
    drawLine(b, c, colour, colour);
    drawLine(c, a, colour, colour);
}

void DebugRenderer::drawContactPoint(const btVector3& point_on_b, const btVector3& normal_on_b,
                                     btScalar distance, int lifeTime, const btVector3& colour)
{
}

void DebugRenderer::reportErrorWarning(const char* warning_string)
{
    std::cerr << "Bullet 3 Warning: " << warning_string << '\n';
}

void DebugRenderer::draw3dText(const btVector3& location, const char* text)
{
}

void DebugRenderer::setDebugMode(int debug_mode)
{
    debug_mode_ = debug_mode;
}

int DebugRenderer::getDebugMode() const
{
    return debug_mode_;
}

void DebugRenderer::render()
{
    drawLine({0, 0, 0}, {0, 10, 0}, {1, 1, 1});
    drawLine({10, 0, 10}, {10, 10, 10}, {1, 1, 1});

    shader_.bind();
    shader_.set_uniform("projection_matrix", p_camera_->get_projection());
    shader_.set_uniform("view_matrix", p_camera_->get_view_matrix());

    mesh_.buffer();
    mesh_.bind();
    mesh_.draw(GL_LINES);

    mesh_.vertices.clear();
    mesh_.indices.clear();
}
