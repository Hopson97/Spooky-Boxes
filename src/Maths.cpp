#include "Maths.h"

glm::mat4 create_model_matrix(const Transform& transform)
{
    glm::mat4 matrix{1.0f};

    matrix = glm::translate(matrix, transform.position);

    matrix = glm::rotate(matrix, glm::radians(transform.rotation.x), {1, 0, 0});
    matrix = glm::rotate(matrix, glm::radians(transform.rotation.y), {0, 1, 0});
    matrix = glm::rotate(matrix, glm::radians(transform.rotation.z), {0, 0, 1});
    return matrix;
}

glm::mat4 create_view_matrix(const Transform& transform, const glm::vec3& up)
{
    auto x_rot = glm::radians(transform.rotation.x);
    auto y_rot = glm::radians(transform.rotation.y);
    glm::vec3 front = {
        glm::cos(y_rot) * glm::cos(x_rot),
        glm::sin(x_rot),
        glm::sin(y_rot) * glm::cos(x_rot),
    };
    glm::vec3 centre = transform.position + glm::normalize(front);

    return glm::lookAt(transform.position, centre, up);
}

glm::mat4 create_projection_matrix(unsigned width, unsigned height, float fov)
{
    auto aspect = static_cast<float>(width) / static_cast<float>(height);
    return glm::perspective(glm::radians(fov), aspect, 0.5f, 500.0f);
}

glm::vec3 forward_vector(const glm::vec3& rotation)
{
    float yaw = glm::radians(rotation.y);
    float pitch = glm::radians(rotation.x);

    return {
        glm::cos(yaw) * glm::cos(pitch),
        glm::sin(pitch),
        glm::cos(pitch) * glm::sin(yaw),
    };
}

glm::vec3 backward_vector(const glm::vec3& rotation)
{
    return -forward_vector(rotation);
}

glm::vec3 left_vector(const glm::vec3& rotation)
{
    float yaw = glm::radians(rotation.y + 90);
    return {
        -glm::cos(yaw),
        0,
        -glm::sin(yaw),
    };
}

glm::vec3 right_vector(const glm::vec3& rotation)
{
    return -left_vector(rotation);
}
