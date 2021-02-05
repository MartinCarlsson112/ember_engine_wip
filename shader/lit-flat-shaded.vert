#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform camera_data
{
	mat4 perspective;
    mat4 view;
    mat4 view_inverse;
    mat4 perspective_inverse;
    vec4 cam_pos;
};


layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 color;
layout(location = 1) out vec3 view_pos;
layout(location = 2) out vec3 frag_pos;


layout(push_constant) uniform indices {
    mat4 mvp;
};


void main() {

    gl_Position = mvp * in_pos;
    view_pos = vec3(cam_pos);
    color = in_color;
    frag_pos = vec3(in_pos);
}