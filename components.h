#pragma once
struct position
{
	float x, y, z;
};

struct velocity
{
	float x, y, z;
};

struct renderable
{
	VkBuffer vbo;
	uint32_t vert_count;
	uint32_t material;
	uint32_t pipeline;
	uint32_t desc;
	uint32_t vertex_stride;
	bool instanced;
};

struct sprite
{
	uint16_t sprite_index;
	uint16_t texture_index;
};

struct scale
{
	float val;
};

struct rotation
{
	float val;
};

struct point_light
{
	float4 color;
};

struct animation
{
	float time;
	uint32_t animation_clip;
	uint32_t rig;
};

struct directional_light
{
	float4 color;
	float4 direction;
};

struct tag
{

};


struct aabb
{
	float3 lower_bound;
	float3 upper_bound;
};

struct rigidbody
{

};


struct static_tag
{

};

struct dynamic_tag
{

};

