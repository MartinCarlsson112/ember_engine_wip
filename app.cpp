#include "app.h"
#include <iostream>
#include "game_app.h"
#include <windowsx.h>
LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	window_manager* window_manager = (struct window_manager*)GetWindowLongPtr(window, GWLP_USERDATA);
	switch (msg)
	{
	case WM_CREATE:
	{
		CREATESTRUCT* createStruct = (CREATESTRUCT*)lparam;
		window_manager = (struct window_manager*)createStruct->lpCreateParams;
		SetWindowLongPtrA(window, GWLP_USERDATA, (LONG_PTR)window_manager);
		window_manager->send_message(window_message::create);
	} break;

	case WM_DESTROY:
	{
		window_manager->send_message(window_message::destroy);
		PostQuitMessage(0);
		game_app::running = false;
	} break;

	case WM_SIZE:
	{
		GetWindowRect(window, &window_manager->wr);
		AdjustWindowRect(&window_manager->wr, 0, FALSE);
		window_manager->send_message(window_message::resize);
	} break;
	case WM_KEYDOWN:
	{
		//if last state is true, ignore this
		if ((lparam) & (1 << (30)))
		{
			window_manager->input_manager.receive_input(input_type::repeat, (unsigned int)wparam);
		}
		else
		{
			window_manager->input_manager.receive_input(input_type::key_down, (unsigned int)wparam);
		}
	}break;
	case WM_KEYUP:
	{
		window_manager->input_manager.receive_input(input_type::key_up, (unsigned int)wparam);
	}break;
	case WM_SYSKEYDOWN:
	{
		window_manager->input_manager.receive_input(input_type::key_down, (unsigned int)wparam);
	}break;
	case WM_SYSKEYUP:
	{
		window_manager->input_manager.receive_input(input_type::key_up, (unsigned int)wparam);
	}break;
	case WM_MOUSEMOVE:
	{
		window_manager->input_manager.set_mouse_pos(int2{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) });
	}break;
	case WM_MOUSEHWHEEL:
	{
		//Input::HandleInput(INPUT_MOUSEWHEEL, (unsigned int)wparam);
	}break;

	case WM_CLOSE:
	{
		DestroyWindow(window);
	}break;

	default:
	{
		return DefWindowProc(window, msg, wparam, lparam);
	}
	}
	return 0;
}


void window_manager::create(size_t width, size_t height, std::wstring name)
{
	locked_mouse = false;
	wr.right = (LONG)width;
	wr.bottom = (LONG)height;

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	window = CreateWindowEx(
		NULL,
		wc.lpszClassName,
		name.c_str(),
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		wr.right,
		wr.bottom,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		this);
	LONG lStyle = GetWindowLong(window, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
	SetWindowLong(window, GWL_STYLE, lStyle);
	hdc = GetDC(window);
	GetWindowRect(window, &wr);
	AdjustWindowRect(&wr, 0, FALSE);
	ShowWindow(window, SW_SHOW);
}

void window_manager::release()
{
	DestroyWindow(window);
}

void window_manager::process_events()
{
	input_manager.reset();

	MSG msg;
	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (locked_mouse)
	{
		int x = wr.right;
		int y = wr.bottom;

		int2 locked_pos = int2(x / 2, y / 2);

		set_cursor_position(locked_pos);
	}

	while (ShowCursor(false) >= 0);

	if (!full_screen)
	{
		if (input_manager.key_up(0x0D))
		{
			full_screen = true;
			ShowWindow(window, SW_MAXIMIZE);
		}
	}
	else
	{
		if (input_manager.key_up(0x0D))
		{
			full_screen = false;
			ShowWindow(window, SW_SHOWDEFAULT);
			
		}
	}

}

void window_manager::process_messages()
{
}

void window_manager::send_message(window_message message)
{
	switch (message)
	{
	case window_message::create:
		break;
	case window_message::destroy:
		break;
	case window_message::resize:
		break;
	default:
		break;
	}
}

void window_manager::add_listener()
{
}

void window_manager::remove_listener()
{
}

void window_manager::set_cursor_position(const int2 pos)
{
	POINT point = { pos.x, pos.y };
	ClientToScreen(window, &point);
	SetCursorPos(pos.x, pos.y);
}

void window_manager::set_cursor_locked(bool value)
{
	locked_mouse = value;
}

void window_manager::set_cursor_locked(bool value, int2 pos)
{
	locked_mouse = value;
	locked_mouse_pos = pos;

}
