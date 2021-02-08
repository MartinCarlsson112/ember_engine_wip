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
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec2 in_uv;


layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 view_pos;
layout(location = 3) out vec3 frag_pos;


layout(push_constant) uniform indices {
    mat4 model;
};


void main() {
 
    view_pos = vec3(cam_pos);

    normal =  -vec3(in_normal) * mat3(transpose(inverse(model)));
    uv = in_uv;
    frag_pos = vec3(model * in_pos);
    gl_Position = perspective * view * model * in_pos;
}