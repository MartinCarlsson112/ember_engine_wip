#pragma once
#include "ecs.h"
#include "components.h"
#include "animation.h"
#include "mesh_batch.h"
inline constexpr float4x4 correction = { 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0.5f,0.5f,0, 0, 0,1 };

struct animation_system
{
	std::vector<clip>* clips;
	std::vector<rig>* rigs;
	pose p;
	void initialize(std::vector<clip>* clips, std::vector<rig>* rigs)
	{
		this->clips = clips;
		this->rigs = rigs;
		p  = rigs->operator[](0).rest_pose;
	}

	component_id_array<animation> comps;
	void update(entity_component_system* ecs, float dt, std::vector<float4x4>& poses)
	{
		poses.clear();
		auto view = ecs->get_view(comps.arr, comps.size);
		auto animations = ecs->get_component_array<animation>();

		for (auto g : *view)
		{
			auto animation_offset = animations + g->get_offset(component_id<animation>);
			for (auto i : *g)
			{
				auto& animation = animation_offset[i];
			
				std::vector<float4x4> matrices;
				animation.time = clips->operator[](animation.animation_clip).sample(p, animation.time + dt * 0.1f);
 				p.get_matrices(matrices);

				std::vector<float4x4>& inv_bind_pose = rigs->operator[](animation.rig).inv_bind_pose;

				poses.resize(matrices.size());
				for (int j = 0; j < matrices.size(); j++)
				{
					poses[j] = matrices[j] * inv_bind_pose[j];
				}
			}
		}
	}

};