#pragma once
#include "ecs.h"
#include "renderer.h"
#include "components.h"
#include "uniform_buffer_object.h"
struct light_system
{
	component_id_array<position, directional_light> comps;
	component_id_array<position, point_light> p_lights_comps;
	position* positions;
	directional_light* lights;
	point_light* p_lights;
	view* light_view;
	view* p_light_view;
	light_buffer_object lbo;

	float accumulate = 0.0f;

	void initialize(entity_component_system* ecs)
	{

	}

	void update(entity_component_system* ecs, float dt)
	{
		light_view = ecs->get_view(comps.arr, comps.size);
		positions = ecs->get_component_array<position>();
		p_lights = ecs->get_component_array<point_light>();
		lights = ecs->get_component_array<directional_light>();
		size_type j = 0;
		size_t index = 0;

		accumulate += dt* 0.05f;

		for (auto g : *light_view)
		{
			auto position_offset = positions + g->get_offset(component_id<position>);
			auto light_os = g->get_offset(component_id<directional_light>);
			auto light_offset = lights + light_os;

			for (auto i : *g)
			{
				auto& pos = position_offset[i];
				auto& light = light_offset[i];

				float4x4 rot_matrix;
				math::rotation_matrix(fmod(accumulate, 360.0f), float3(1, 1, 0), rot_matrix);
				float4x4 inverted;
				bool valid = math::inverse_matrix(rot_matrix, inverted);

				light.direction = float4(math::normalize(float3(inverted[8], inverted[9], inverted[10])), 0.0f);

				lbo.lights[index] = light_data{ float4(pos.x, pos.y, pos.z, 1.0f), light.color, light.direction};
				index++;
			}
		}

		index = 0;
		p_light_view = ecs->get_view(p_lights_comps.arr, p_lights_comps.size);

		for (auto g : *p_light_view)
		{
			auto position_offset = positions + g->get_offset(component_id<position>);
			auto light_os = g->get_offset(component_id<point_light>);
			auto light_offset = p_lights + light_os;

			for (auto i : *g)
			{
				auto& pos = position_offset[i];
				auto& light = light_offset[i];
				
				lbo.point_lights[index] = point_light_data{ float4(pos.x, pos.y, pos.z, 1.0f), light.color };
				index++;
			}
		}

	}


};