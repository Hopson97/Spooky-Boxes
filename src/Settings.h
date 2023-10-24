#pragma once

#include "Graphics/Lights.h"

struct Settings
{
    Settings()
    {
        lights.dir_light.direction = {0.8f, -0.2f, 0.1f, 0.0f};
        lights.dir_light.ambient_intensity = 0.2f;
        lights.dir_light.diffuse_intensity = 0.6f;
        lights.dir_light.specular_intensity = 0.0f;

        lights.point_light.ambient_intensity = 0.3f;
        lights.point_light.diffuse_intensity = 1.0f;
        lights.point_light.specular_intensity = 1.0f;
        lights.point_light.att.constant = 1.0f;
        lights.point_light.att.linear = 0.045f;
        lights.point_light.att.exponant = 0.0075f;

        lights.spot_light.ambient_intensity = 0.012f;
        lights.spot_light.diffuse_intensity = 0.35f;
        lights.spot_light.specular_intensity = 1.0f;
        lights.spot_light.att.constant = 0.2f;
        lights.spot_light.att.linear = 0.016f;
        lights.spot_light.att.exponant = 0.003f;
        lights.spot_light.cutoff = 50.0f;
    }
    struct Light
    {
        DirectionalLight dir_light;
        PointLight point_light;
        SpotLight spot_light;
    } lights;

    float material_shine = 32.0f;

    bool grass = true;

    bool wireframe = false;

    float throw_force = 4000.0f;
    float throw_mass = 1.0f;
};