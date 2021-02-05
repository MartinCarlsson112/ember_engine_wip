#pragma once
#include "app.h"
#include "renderer.h"
#include "camera_system.h"
#include "render_system.h"
#include "light_system.h"
#include "resource_manager.h"
#include <chrono>
struct game_app_data
{
	//ecs
};


struct game_app
{
	typedef std::chrono::nanoseconds nano_seconds;
	typedef std::chrono::steady_clock::time_point time_point;
	typedef std::chrono::high_resolution_clock clock;

	void initialize();
	void release();
	
	void start();
	void update(float dt);

	void dispose();
	static bool running;

	bool locked_mouse;

	window_manager wm;
	renderer render;
	entity_component_system ecs;
	int2 window_size;
	int2 window_center;


	resource_manager resources;
	camera_system camera_sys;
	render_system render_sys;
	light_system light_sys;
	time_point base_time;

	nano_seconds target_time;
	nano_seconds lag;
};


