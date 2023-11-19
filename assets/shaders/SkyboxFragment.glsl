#version 330

layout(location = 0) out vec4 out_colour;

in vec3 pass_texture_coord;

uniform samplerCube cube_sampler;

void main() {
    out_colour = texture(cube_sampler, pass_texture_coord);
}