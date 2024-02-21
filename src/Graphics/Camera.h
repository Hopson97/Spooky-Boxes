#pragma once

#include "../Utils/Maths.h"

struct PerspectiveCamera
{
  public:
    Transform transform;

  public:
    PerspectiveCamera(unsigned window_size, unsigned window_width, float fov);
    void update();
    void gui();

    const glm::mat4& get_view_matrix() const;
    const glm::mat4& get_projection() const;
    const glm::vec3& get_forwards() const;

  private:
    glm::mat4 projection_matrix_{1.0f};
    glm::mat4 view_matrix_{1.0f};
    glm::vec3 forwards_{0.0f};

    float aspect_ = 0;
    float fov_ = 0;
    float near_ = 1.0f;
    float far_ = 20000.0f; 
};