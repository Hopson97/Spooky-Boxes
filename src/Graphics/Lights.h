#pragma once

#include <glm/glm.hpp>

struct LightBase
{
    glm::vec4 colour = {1, 1, 1, 0};
    float ambient_intensity = 0.2f;
    float diffuse_intensity = 0.2f;
    float specular_intensity = 0.2f;
    float padding_ = 0.0f;
};

struct Attenuation
{
    float constant = 1.0f;
    float linear = 0.045f;
    float exponant = 0.0075f;
};

struct DirectionalLight : public LightBase
{
    glm::vec4 direction = {0, -1, 0, 0.0f};
};

struct PointLight : public LightBase
{
    glm::vec4 position = {0, 0, 0, 0};
    Attenuation att;
    float padding_ = 0.0f;
};

struct SpotLight : public LightBase
{
    glm::vec4 direction = {1, 0, 0, 0};
    glm::vec4 position = {0, 0, 0, 0};
    Attenuation att;

    float cutoff = 2.0f;
};