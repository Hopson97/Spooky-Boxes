#version 330 core

out vec4 out_colour;

in vec4 pass_colour;

void main()
{    
    out_colour = pass_colour;
}  