#pragma once

#include "input.h"
#include "mmath.h"
#include "uniform_buffer_object.h"
#include <iostream>
struct camera_system
{
	float4x4 vp;

	camera_data cam_data;

	float3 camera_pos = float3(0, 0, -1);
	float3 camera_dir = float3(0, 0, 1);
	float3 camera_up = float3(0, 1, 0);
	float3 cam_rot = float3(0, 0, 0);

	float3 camera_right = float3(1, 0, 0);
	const float3 WORLD_UP = float3(0, 1, 0);

	float fly_speed = .2f;
	float turn_speed = 1.0f;

	void update_vp()
	{
		math::mul(cam_data.perspective, cam_data.view, vp);
		bool valid = math::inverse_matrix(cam_data.view, cam_data.view_inverse);
	}

	void initialize(int2 window_size)
	{
		float aspectRatio = (float)window_size.x / (float)window_size.y;
		math::perspective(60, aspectRatio, 0.01f, 100.0f, cam_data.perspective);
		bool valid = math::inverse_matrix(cam_data.perspective, cam_data.proj_inverse);

		math::lookat_matrix(camera_pos, camera_right, camera_dir, camera_up, cam_data.view);
		update_vp();
	}

	void fps_camera_update(float dt, input_manager im, int2 window_center)
	{
		float3 move_dir = float3();

		if (im.key(0x57))
		{
			move_dir += camera_dir;
		}
		if (im.key(0x53))
		{
			move_dir += -camera_dir;
		}
		if (im.key(0x44))
		{
			move_dir += -camera_right;
		}
		if (im.key(0x41))
		{
			move_dir += camera_right;
		}

		if (move_dir.x != 0 || move_dir.y != 0 || move_dir.z != 0)
		{
			camera_pos += math::normalize(move_dir) * fly_speed * dt;
		}

		auto mouse_pos = im.mouse_pos();
		float center_x = (float)window_center.x;
		float center_y = (float)window_center.y;

		cam_rot += math::deg2rad(float3((center_x - mouse_pos.x), (center_y - mouse_pos.y), 0) * turn_speed * dt);
		cam_rot.y = math::clamp(cam_rot.y, -math::pi / 2.0f + 0.1f, math::pi / 2.0f - 0.1f);
		float4x4 rot_matrix;
		math::rotation_matrix(cam_rot.y, float3(1.0f, 0.0f, 0.0f), rot_matrix);

		float4x4 rot_matrix2;
		math::rotation_matrix(cam_rot.x, float3(0.0f, 1.0f, 0.0f), rot_matrix2);

		float4x4 final_rot_matrix;
		math::mul(rot_matrix, rot_matrix2, final_rot_matrix);


		float4x4 translation_matrix;
		math::translate(camera_pos, translation_matrix);
		math::mul(final_rot_matrix, translation_matrix, cam_data.view);

		float4x4 inverted;
		bool valid = math::inverse_matrix(cam_data.view, inverted);
		camera_dir = math::normalize(float3(inverted[8], inverted[9], inverted[10]));
		camera_right = math::cross(camera_dir, -WORLD_UP);
		update_vp();
	}
};