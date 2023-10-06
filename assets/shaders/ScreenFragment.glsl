#version 450

layout (location = 0) out vec4 out_colour;

in vec2 pass_texture_coord;

uniform sampler2D texture_Colour;

void main() {
    out_colour = texture(texture_Colour, pass_texture_coord);
    //out_colour.r = 1.0;
   // float depth = texture(colourTexture, passTexCoord).r;
    //depth = 1.0 - (1.0 - depth) * 50.0;
    //outColour = vec4(depth);
}