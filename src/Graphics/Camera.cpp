#include "Camera.h"

PerspectiveCamera::PerspectiveCamera(unsigned width, unsigned height, float fov)
{
    auto aspect = static_cast<float>(width) / static_cast<float>(height);
    projection_matrix_ = glm::perspective(glm::radians(fov), aspect, 0.5f, 500.0f);
}

void PerspectiveCamera::update()
{
    auto x_rot = glm::radians(transform.rotation.x);
    auto y_rot = glm::radians(transform.rotation.y);
    forwards_ = {
        glm::cos(y_rot) * glm::cos(x_rot),
        glm::sin(x_rot),
        glm::sin(y_rot) * glm::cos(x_rot),
    };
    glm::vec3 centre = transform.position + glm::normalize(forwards_);

    view_matrix_ = glm::lookAt(transform.position, centre, {0, 1, 0});
}

const glm::mat4& PerspectiveCamera::get_view_matrix() const
{
    return view_matrix_;
}

const glm::mat4& PerspectiveCamera::get_projection() const
{
    return projection_matrix_;
}

const glm::vec3& PerspectiveCamera::get_forwards() const
{
    return forwards_;
}