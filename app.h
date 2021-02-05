#pragma once
#include <string>
#include <Windows.h>
#include "input.h"
#include <cstdint>
enum class window_message{
	create,
	destroy,
	resize
};

struct window_manager
{
	void create(size_t width, size_t height, std::wstring name);
	void release();
	void process_events();
	void set_cursor_position(const int2 pos);

	void set_cursor_locked(bool value, int2 pos);
	void set_cursor_locked(bool value);

	void process_messages();
	void send_message(window_message message);

	void add_listener();
	void remove_listener();

	struct input_manager input_manager;

	bool locked_mouse = false;
	int2 locked_mouse_pos;

	bool full_screen = false;
	RECT wr;
	HWND window;
	HDC hdc;
	//queue of messages
	//listeners
};
