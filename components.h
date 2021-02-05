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
	uint32_t shader;
	uint32_t albedo;
	uint32_t material;



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

struct aabb
{
	float3 min, max;
};

struct point_light
{
	float4 color;
};


struct directional_light
{
	float4 color;
	float4 direction;
};