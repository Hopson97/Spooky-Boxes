#version 450 core

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_albedo_spec;

in vec2 pass_texture_coord;
in vec3 pass_normal;
in vec3 pass_fragment_coord;

struct Material 
{
    sampler2D diffuse0;
    sampler2D specular0;
};

uniform Material material;


void main() {
	out_position = pass_fragment_coord;
	out_normal = normalize(pass_normal);
	out_albedo_spec.rgb = texture(material.diffuse0, pass_texture_coord).rgb;
	out_albedo_spec.a = texture(material.specular0, pass_texture_coord).a;
}