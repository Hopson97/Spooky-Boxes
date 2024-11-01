#version 450 core

#define MAX_LIGHTS 64

layout (location = 0) out vec4 out_colour;

in vec2 pass_texture_coord;

// in vec3 pass_normal;
// in vec3 pass_fragment_coord;

// The 'pass_fragment_coord' aka position of the fragment in the world
uniform sampler2D postion_tex;
uniform sampler2D normal_tex;
uniform sampler2D albedo_spec_tex;

struct LightBase 
{
    vec3 colour;
    float ambient_intensity;
    float diffuse_intensity;
    float specular_intensity;
};

struct Attenuation
{
    float constant;
    float linear;
    float exponant;
};

struct DirectionalLight 
{
    LightBase base;
    vec3 direction;
};

struct PointLight 
{
    LightBase base;
    Attenuation att;
    vec3 position;
};

struct SpotLight 
{
    LightBase base;
    Attenuation att;
    vec3 direction;
    vec3 position;

    float cutoff;
};

uniform DirectionalLight dir_light;
uniform SpotLight spot_light;
uniform PointLight point_lights[MAX_LIGHTS];
uniform int light_count;

uniform bool is_light;
uniform vec3 eye_position;



/**
    Calculates the base lighting 

    @param light The base light object 
    @param normal The vertex normal 
    @light_direction The direction from the surface to the light 
    @light_direction The direction from the camera's "eye" to the light 

    @return Combined light effect (Ambient + Diffuse + Specular)
*/
vec3 calculate_base_lighting(LightBase light, vec3 normal, vec3 light_direction, vec3 eye_direction)
{
    float specular = texture(normal_tex, pass_texture_coord).a;

    vec3 ambient_light = light.colour * light.ambient_intensity;

    // Diffuse lighting
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = light.colour * light.diffuse_intensity * diff;

    // Specular lighting
    vec3 reflect_direction  = reflect(-light_direction, normal);
    float spec              = pow(max(dot(eye_direction, reflect_direction), 0.0),32.0f);// material.shininess);
    float specular_total           = light.specular_intensity * spec * specular;

    return ambient_light + diffuse + specular_total;
}

/**
    Calculates attenuation for the light from the fragment position

    @param attenuation Attenuation values to calculate from
    @param light_position The position of the light source

    @return Attenuation intensity (between 0 and 1), multiply the light by this
*/
float calculate_attenuation(Attenuation attenuation, vec3 light_position)
{
    vec3 fragment_coord = texture(postion_tex, pass_texture_coord).rgb;

    // Attenuation
    float distance = length(light_position - fragment_coord);
    return 1.0 /  (
        attenuation.constant + 
        attenuation.linear * distance + 
        attenuation.exponant * (distance * distance)
    );
}

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 eye_direction)
{
    return calculate_base_lighting(light.base, normalize(-light.direction), normal, eye_direction);
}

vec3 calculate_point_light(PointLight light, vec3 normal, vec3 eye_direction) 
{
    vec3 fragment_coord = texture(postion_tex, pass_texture_coord).rgb;

    vec3 light_result = calculate_base_lighting(light.base, normalize(light.position - fragment_coord), normal, eye_direction);
    float attenuation = calculate_attenuation(light.att, light.position);

    return light_result * attenuation;
}

vec3 calculate_spot_light(SpotLight light, vec3 normal, vec3 eye_direction) 
{
    vec3 fragment_coord = texture(postion_tex, pass_texture_coord).rgb;
    
    vec3 light_direction = normalize(light.position - fragment_coord);
    vec3 light_result = calculate_base_lighting(light.base, light_direction, normal, eye_direction);

    float attenuation = calculate_attenuation(light.att, light.position);

    // Smooth edges, creates the flashlight effect such that only centre pixels are lit
    float oco = cos(acos(light.cutoff) + radians(6));
    float theta = dot(light_direction, -light.direction);
    float epsilon = light.cutoff - oco;
    float intensity = clamp((theta - oco) / epsilon, 0.0, 1.0);
    
    // Apply the attenuation and the flashlight effect. Note the flashlight also effects
    // this light source's ambient light, so this will only allow light inside the "light cone" 
    // - this may need to be changed
    return light_result * intensity * attenuation;

}

void main()
{
    vec3 fragment_coord = texture(postion_tex, pass_texture_coord).rgb;
    vec3 normal_in = texture(normal_tex, pass_texture_coord).rgb;
    out_colour = vec4(texture(albedo_spec_tex, pass_texture_coord).rgb, 1.0);


    if (is_light)
    {
        out_colour *= 2.0f;
        return;
    }
    vec3 normal = normalize(normal_in);
    vec3 eye_direction = normalize(eye_position - fragment_coord); 

    vec3 total_light = vec3(0, 0, 0);
    total_light += calculate_directional_light(dir_light, normal, eye_direction);

    for (int i = 0; i < light_count; i++) {
        total_light += calculate_point_light(point_lights[i], normal, eye_direction);
    }
    total_light += calculate_spot_light(spot_light, normal, eye_direction);

    out_colour *= vec4(total_light, 1.0);

    out_colour = clamp(out_colour, 0, 1);
}
