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

layout(set = 0, binding = 3) uniform animation_data
{
    mat4 pose[120];
};

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in ivec4 in_bones;
layout(location = 4) in vec4 in_weights;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 view_pos;
layout(location = 3) out vec3 frag_pos;

layout(push_constant) uniform indices {
    mat4 model;
};

void main() {

    mat4 skin = pose[in_bones.x] * in_weights.x; +  pose[in_bones.y] * in_weights.y +  pose[in_bones.z] * in_weights.z + pose[in_bones.w] * in_weights.w;
    vec4 p = vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
    view_pos = vec3(cam_pos);
    normal =  mat3(model)* mat3(skin) * vec3(in_normal);

    uv = in_uv;
    frag_pos = vec3(model * skin * p);


    gl_Position = perspective * view * model* skin * p;
}