#pragma once

#include "mmath.h"

struct light_data
{
	float4 pos;
	float4 color;
	float4 dir;
};

struct point_light_data
{
	float4 pos;
	float4 color;
};

inline constexpr size_t LIGHT_COUNT = 5;

struct light_buffer_object
{
	light_data lights[LIGHT_COUNT];
	point_light_data point_lights[LIGHT_COUNT];
};

struct camera_data
{
	float4x4 perspective;
	float4x4 view;
	float4x4 view_inverse;
	float4x4 proj_inverse;
	float4 view_pos;
};