#include "input.h"

void input_manager::receive_input(const input_type type, const uint32_t key)
{
    switch (type)
    {
    case input_type::key_down:
    {
        input_data.keys[key] = static_cast<uint8_t>(input_type::key_down);
    }
        break;
    case input_type::key_up:
    {
        input_data.keys[key] = static_cast<uint8_t>(input_type::key_up);
    }
        break;
    case input_type::repeat:
    {
        input_data.keys[key] = static_cast<uint8_t>(input_type::repeat);
    }
        break;
    default:

        break;
    }
}

void input_manager::set_mouse_pos(const int2& pos)
{
    input_data.mouse.pos = pos;
}

void input_manager::set_mouse_wheel(const float value)
{
	input_data.mouse.wheel = value;
}


bool input_manager::key_down(const uint32_t key)const
{
    return input_data.keys[key] == (static_cast<uint8_t>(input_type::key_down));
}

bool input_manager::key_up(const uint32_t key) const
{
	return input_data.keys[key] == (static_cast<uint8_t>(input_type::key_up));
}

bool input_manager::key(const uint32_t key)const
{
	return input_data.keys[key] == (static_cast<uint8_t>(input_type::key_down)) || 
            input_data.keys[key] == (static_cast<uint8_t>(input_type::repeat));
}

int2 input_manager::mouse_pos()const
{
    return input_data.mouse.pos;
}

float input_manager::mouse_wheel() const
{
	return input_data.mouse.wheel;
}

void input_manager::reset()
{
    for (int i = 0; i < 512; i++)
    {
        if (input_data.keys[i] == (static_cast<uint8_t>(input_type::key_down)))
        {
            input_data.keys[i] = (static_cast<uint8_t>(input_type::repeat));
        }
		if (input_data.keys[i] == (static_cast<uint8_t>(input_type::key_up)))
		{
			input_data.keys[i] = (static_cast<uint8_t>(input_type::none));
		}

    }
}
