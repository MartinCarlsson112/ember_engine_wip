#pragma once
#include "ecs.h"
#include "components.h"
#include "animation.h"
#include "mesh_batch.h"
struct animation_system
{
	std::vector<clip>* clips;
	std::vector<rig>* rigs;

	void initialize(std::vector<clip>* clips, std::vector<rig>* rigs)
	{
		this->clips = clips;
		this->rigs = rigs;
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
				animation.time += dt;

				pose p = rigs->operator[](animation.rig).rest_pose;
				animation.time = clips->operator[](animation.animation_clip).sample(p, animation.time);

				std::vector<float4x4> matrices;

				p.get_matrices(matrices);

				poses.resize(matrices.size());
				for (int j = 0; j < matrices.size(); j++)
				{
					math::mul(matrices[i], rigs->operator[](0).inv_bind_pose[j], poses[j]);
				}
			}
		}
	}

};