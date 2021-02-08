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

    mat4 skin = mat4(1.0);
    if(in_bones.x != -1)
    {
        skin = pose[in_bones.x] * in_weights.x;
    }
    if(in_bones.y != -1)
    {
       skin += pose[in_bones.y] * in_weights.y;
    }
    if(in_bones.z != -1)
    {
      skin += pose[in_bones.z] * in_weights.z;
    }
    if(in_bones.w != -1)
    {
        skin += pose[in_bones.w] * in_weights.w;
    }

    view_pos = vec3(cam_pos);
    normal =  -vec3(in_normal) * mat3(skin) * mat3(transpose(inverse(model)));

    uv = in_uv;
    frag_pos = vec3(model * skin * in_pos);
    gl_Position = perspective * view * model* skin * in_pos;
}