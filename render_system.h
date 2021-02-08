#pragma once
#include "ecs.h"
#include <vector>
#include "components.h"
#include "mmath.h"
#include "camera_system.h"
#include "renderer.h"

constexpr double scaleFactor = 65530.0;
constexpr double cp = 256.0 * 256.0;

//inject renderable hash
//for now just use simple xor to create hash.

namespace std {

	template <>
	struct hash<renderable>
	{
		std::size_t operator()(const renderable& k) const
		{
			using std::size_t;
			using std::hash;
			using std::string;

			return ((hash<uint32_t>()(k.material)
				^ (hash<VkBuffer>()(k.vbo) << 1)) >> 1);
		}
	};

}

//comparison operator for renderable
inline bool operator==( const renderable& rhs,  const renderable& lhs)
{
	return (rhs.vbo == lhs.vbo) &&
		(rhs.material == lhs.material);
}

struct render_system
{
	
	view* renderable_view;
	component_id_array<position, renderable> comps;

	std::vector<mesh_batch> batches;
	std::unordered_map<renderable, uint32_t> batch_indexing;

	void gather_renderables(renderer* render, entity_component_system* ecs, const float4x4& vp)
	{
		std::vector<float4x4> mvps;
		position* positions = ecs->get_component_array<position>();
		renderable* renderables = ecs->get_component_array<renderable>();
		renderable_view = ecs->get_view(comps.arr, comps.size);
		size_type j = 0;
		uint8_t index = 0;

		//check if needs update

		for (auto& batch : batches)
		{
			batch.count = 0;
		}

		for (auto g : *renderable_view)
		{
			auto positionOffset = positions + g->get_offset(component_id<position>);
			auto renderableOffset = renderables + g->get_offset(component_id<renderable>);

			for (auto i : *g)
			{
				auto& pos = positionOffset[i];
				auto& rend = renderableOffset[i];

				if (batch_indexing.find(rend) != batch_indexing.end())
				{
					auto batch_index = batch_indexing[rend];
					auto batch_count = batches[batch_index].count++;
					if (batch_count < MAX_BATCHED_MESHES_COUNT)
					{
						math::translate(float3(pos.x, pos.y, pos.z), batches[batch_index].model[batch_count]);
					}
				}
				else
				{
					batches.push_back(mesh_batch());
					auto& batch = batches[batches.size() - 1];
					batch.count = 1;
					batch.material = rend.material;
					batch.vbo = rend.vbo;
					batch.vertex_count = rend.vert_count;
					batch.descriptor_set = rend.desc;
					batch.pipeline = rend.pipeline;
					batch.vertex_stride = rend.vertex_stride;
					math::translate(float3(pos.x, pos.y, pos.z), batch.model[0]);
				
					batch_indexing[rend] = (uint32_t)(batches.size() - 1);
				}
			}
		}
	}
};