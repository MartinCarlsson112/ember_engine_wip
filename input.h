#pragma once
#include "mmath.h"
#include "collections.h"
#include <cstdint>

enum class input_type
{
	none,
	key_down,
	key_up,
	repeat,
};

struct input_mouse
{
	int2 pos;
	float wheel;
};

struct input_data
{
	uint8_t keys[512];
	input_mouse mouse;
	//gamepads
};

struct input_listener
{
	//instance
	//func
};

struct input_manager
{
	void receive_input(const input_type type, const uint32_t key);
	void set_mouse_pos(const int2& pos);
	void set_mouse_wheel(const float value);
	bool key_down(const uint32_t key) const;
	bool key_up(const uint32_t key) const ;
	bool key(const uint32_t key) const ;
	int2 mouse_pos() const ;
	float mouse_wheel() const;
	struct input_data input_data;

	void reset();

	dynamic_array<struct input_listener> input_listeners;
};