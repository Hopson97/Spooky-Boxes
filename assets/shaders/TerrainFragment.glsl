#version 450 core

#define MAX_LIGHTS 5

layout (location = 0) out vec4 out_colour;

in vec2 pass_texture_coord;
in vec3 pass_normal;
in vec3 pass_fragment_coord;

struct Material 
{
    sampler2D grass_diffuse;
    sampler2D grass_specular;
    sampler2D mud_diffuse;
    sampler2D mud_specular;

    sampler2D snow_diffuse;
    sampler2D snow_specular;
};

struct LightBase 
{
    vec4 colour;
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
    vec4 direction;
};

struct PointLight 
{
    LightBase base;
    vec4 position;
    Attenuation att;
};

struct SpotLight 
{
    LightBase base;
    vec4 direction;
    vec4 position;
    Attenuation att;

    float cutoff;
};

layout(std140) uniform Light 
{
    DirectionalLight dir_light;
    SpotLight spot_light;
};


layout(std140) uniform PointLights 
{
    PointLight point_lights[MAX_LIGHTS];
    
};

uniform Material material;
uniform int light_count;
uniform vec3 eye_position;

uniform float max_height;



/**
    Calculates the base lighting 

    @param light The base light object 
    @param normal The vertex normal 
    @light_direction The direction from the surface to the light 
    @light_direction The direction from the camera's "eye" to the light 

    @return Combined light effect (Ambient + Diffuse + Specular)
*/
vec3 calculate_base_lighting(LightBase light, vec3 normal, vec3 light_direction, vec3 eye_direction, vec3 specular)
{
    vec3 ambient_light = light.colour.rgb * light.ambient_intensity;

    // Diffuse lighting
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = light.colour.rgb * light.diffuse_intensity * diff;

    // Specular lighting
    vec3 reflect_direction  = reflect(-light_direction, normal);
    float spec              = pow(max(dot(eye_direction, reflect_direction), 0.0), 16.0);//material.shininess);

    return ambient_light + diffuse + light.specular_intensity * spec *specular;// 
}

/**
    Calculates attenuation for the light from the fragment position

    @param attenuation Attenuation values to calculate from
    @param light_position The position of the light source

    @return Attenuation intensity (between 0 and 1), multiply the light by this
*/
float calculate_attenuation(Attenuation attenuation, vec3 light_position)
{
    // Attenuation
    float distance = length(light_position - pass_fragment_coord);
    return 1.0 /  (
        attenuation.constant + 
        attenuation.linear * distance + 
        attenuation.exponant * (distance * distance)
    );
}

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 eye_direction, vec3 specular)
{
    return calculate_base_lighting(
        light.base, normalize(-light.direction.xyz), normal, eye_direction, specular
    );
}

vec3 calculate_point_light(PointLight light, vec3 normal, vec3 eye_direction, vec3 specular) 
{
    vec3 light_result = calculate_base_lighting(
        light.base, normalize(light.position.xyz - pass_fragment_coord), normal, eye_direction, specular
    );
    float attenuation = calculate_attenuation(light.att, light.position.xyz);
    return light_result * attenuation;
}

vec3 calculate_spot_light(SpotLight light, vec3 normal, vec3 eye_direction, vec3 specular) 
{
    
    vec3 light_direction = normalize(light.position.xyz - pass_fragment_coord);
    vec3 light_result = calculate_base_lighting(light.base, light_direction, normal, eye_direction, specular);

    float attenuation = calculate_attenuation(light.att, light.position.xyz);
    // Smooth edges, creates the flashlight effect such that only centre pixels are lit
    float oco = cos(acos(light.cutoff) + radians(6));

    float theta = dot(light_direction, -light.direction.xyz);
    float epsilon = light.cutoff - oco;
    float intensity = clamp((theta - oco) / epsilon, 0.0, 1.0);
    
    // Apply the attenuation and the flashlight effect. Note the flashlight also effects
    // this light source's ambient light, so this will only allow light inside the "light cone" 
    // - this may need to be changed
    return light_result * intensity * attenuation;

}

void main()
{

    vec4 base_colour = texture(material.grass_diffuse, pass_texture_coord);
    if (max_height > 100) {
        // Transition values
        float snow_begin = max_height * 0.75;
        float snow_end = max_height * 0.8;

        float fragment_height = pass_fragment_coord.y;


        // Mix transition
        if (fragment_height > snow_end) {
            base_colour = texture(material.snow_diffuse, pass_texture_coord);
        }
        else if (fragment_height > snow_begin) {
            float ratio = (snow_end - fragment_height) / ( snow_end - snow_begin);
            base_colour = 
                mix(
                    texture(material.snow_diffuse, pass_texture_coord),
                    base_colour, 
                    ratio 
                );
        }
    }

    vec3 normal = normalize(pass_normal);

    float base_weight = dot(vec3(0, 1, 0), normalize(normal * vec3(3, 1, 3)));
    float cliff_weight = 1 - base_weight;

    //vec4 base   = texture(material.grass_diffuse, pass_texture_coord);
    vec4 cliff = texture(material.mud_diffuse, pass_texture_coord);

    vec4 base_spec = texture(material.grass_specular, pass_texture_coord);
    vec4 cliff_spec   = texture(material.mud_specular, pass_texture_coord);

    out_colour = cliff * cliff_weight + base_weight * base_colour;

    vec3 specular = vec3(cliff_spec * cliff_weight + base_weight * base_spec);



    vec3 eye_direction = normalize(eye_position - pass_fragment_coord); 
    vec3 total_light = vec3(0, 0, 0);
    total_light += calculate_directional_light(dir_light, normal, eye_direction, specular);

    for (int i = 0; i < light_count; i++) 
    {
        total_light += calculate_point_light(point_lights[i], normal, eye_direction, specular);
    }
    total_light += calculate_spot_light(spot_light, normal, eye_direction, specular);






    out_colour *= vec4(total_light, 1.0);

    out_colour = clamp(out_colour, 0, 1);
}
