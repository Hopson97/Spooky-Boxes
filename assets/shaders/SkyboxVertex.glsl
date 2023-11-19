#version 330

layout(location = 0) in vec3 in_position;

out vec3 pass_texture_coord;

layout(std140) uniform matrix_data {
    mat4 projection_matrix;
    mat4 view_matrix;
};

void main() {
    mat4 view_matrix_no_translation = view_matrix;
    view_matrix_no_translation[3][0] = 0;
    view_matrix_no_translation[3][1] = 0;
    view_matrix_no_translation[3][2] = 0;

    gl_Position = projection_matrix * view_matrix_no_translation * vec4(in_position, 1.0);

    pass_texture_coord = in_position;
}