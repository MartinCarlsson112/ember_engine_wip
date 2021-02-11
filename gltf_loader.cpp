#include "gltf_loader.h"
#pragma warning(disable : 26451)
cgltf_data* load_gltf_file(const char* file_path)
{
	cgltf_options options;    
	memset(&options, 0, sizeof(cgltf_options));   
	cgltf_data* data = NULL;    
	cgltf_result result = cgltf_parse_file(&options, file_path, &data);   
	if (result != cgltf_result_success) {
		//TODO: error logging
		return 0;
	}    
	result = cgltf_load_buffers(&options, data, file_path);

	if (result != cgltf_result_success) { 
		//TODO: error logging
		cgltf_free(data);        
	   
		return 0; 
	}    
	result = cgltf_validate(data);    
	if (result != cgltf_result_success) 
	{ 
		//TODO: error logging
		cgltf_free(data);        
		return 0; 
	}    
	return data;
}

void free_gltf_file(cgltf_data* handle)
{
	if (handle != nullptr)
	{
		cgltf_free(handle);
	}
	else
	{
		//TODO: error logging
	}
}

void mesh_from_attribute(std::vector<skinned_vertex>& vertices, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, size_t node_count)
{
	cgltf_attribute_type attrib_type = attribute.type;
	cgltf_accessor& accessor = *attribute.data;
	size_t component_count = 2;

	if (accessor.type == cgltf_type_vec3)
	{
		component_count = 3;
	}
	else if (accessor.type == cgltf_type_vec4)
	{
		component_count = 4;
	}
	std::vector<float>values;
	get_scalar_values(values, component_count, accessor);
	size_t accessor_count = accessor.count;
	vertices.resize(accessor_count);
	for (size_t i = 0; i < accessor_count; ++i) {
		int index = i * (int)component_count;

		switch (attrib_type)
		{
		case cgltf_attribute_type_position:
		{
			vertices[i].position = float4(values[index + 0], values[index + 1], values[index + 2], 1.0f);
		}break;
		case cgltf_attribute_type_normal:
		{
			float3 normal = float3(values[index + 0], values[index + 1], values[index + 2]);
			if (math::sqr_length(normal) < 0.00001f)
			{
				normal = float3(0, 1, 0);
			}
			vertices[i].normal = normal;
		}break;

		case cgltf_attribute_type_texcoord:
		{
			vertices[i].uvs = float2(values[index+0], values[index + 1]);
		}break;

		case cgltf_attribute_type_weights:
		{
			vertices[i].weights = float4(values[index + 0], values[index + 1], values[index + 2], values[index + 3]);
		}
		break;
		case cgltf_attribute_type_joints:
		{
			int4 bone_ids;
			bone_ids.x = (int)(values[index + 0] + 0.5f);
			bone_ids.y = (int)(values[index + 1] + 0.5f);
			bone_ids.z = (int)(values[index + 2] + 0.5f);
			bone_ids.w = (int)(values[index + 3] + 0.5f);
			bone_ids.x = get_node_index(skin->joints[bone_ids.x], nodes, node_count);
			bone_ids.y = get_node_index(skin->joints[bone_ids.y], nodes, node_count);
			bone_ids.z = get_node_index(skin->joints[bone_ids.z], nodes, node_count);
			bone_ids.w = get_node_index(skin->joints[bone_ids.w], nodes, node_count);
			bone_ids.x = std::max<int>(0, bone_ids.x);
			bone_ids.y = std::max<int>(0, bone_ids.y);
			bone_ids.z = std::max<int>(0, bone_ids.z);
			bone_ids.w = std::max<int>(0, bone_ids.w);
			vertices[i].bones = bone_ids;
		} break;
		}

	}

}

rig load_skeleton(cgltf_data* data)
{
	pose rest = load_rest_pose(data), bind = load_bind_pose(data);
	std::vector<std::string> joint_names = load_joint_names(data);
	return rig(rest, bind, joint_names);
}

pose load_rest_pose(cgltf_data* data)
{
	size_t bone_count = data->nodes_count;
	pose result((uint32_t)bone_count);
	for (size_t i = 0; i < bone_count; ++i)
	{
		cgltf_node* node = &(data->nodes[i]);
		transform t = get_local_transform(data->nodes[i]);
		result.set_local_transform(i, t);
		int parent = get_node_index(node->parent, data->nodes, bone_count);
		result.set_parent(i, parent);
	}
	return result;
}

pose load_bind_pose(cgltf_data* data)
{
	pose rest_pose = load_rest_pose(data);
	size_t num_bones = rest_pose.size();
	std::vector<transform> world_bind_pose(num_bones);
	for (size_t i = 0; i < num_bones; ++i)
	{
		world_bind_pose[i] = rest_pose.get_global_transform(i);
	}
	size_t num_skins = data->skins_count;
	for (size_t i = 0; i < num_skins; ++i)
	{
		cgltf_skin* skin = &(data->skins[i]);
		std::vector<float> inv_bind_accessor;
		get_scalar_values(inv_bind_accessor, 16, *skin->inverse_bind_matrices);
		size_t num_joints = skin->joints_count;
		for (int j = 0; j < num_joints; ++j)
		{
			float* matrix = &(inv_bind_accessor[j * 16]);

			float4x4 inv_bind_matrix;
			float4x4 bind_matrix;
			memcpy(inv_bind_matrix.data(), matrix, 16 * sizeof(float));

			math::inverse_matrix(inv_bind_matrix, bind_matrix);
			transform bind_transform = math::to_transform(bind_matrix);

			cgltf_node* jointNode = skin->joints[j];
			int joint_index = get_node_index(jointNode, data->nodes, num_bones);
			world_bind_pose[joint_index] = bind_transform;
		}

	}

	pose bind_pose = rest_pose;
	for (size_t i = 0; i < num_bones; ++i)
	{
		transform current = world_bind_pose[i];
		int p = bind_pose.get_parent(i);
		if (p >= 0)
		{
			//Bring into parent space            
			transform parent = world_bind_pose[p];
			current = math::combine(math::inverse(parent), current);
		}
		bind_pose.set_local_transform(i, current);
	}
	return bind_pose;
}



transform get_local_transform(cgltf_node& n)
{
	transform result;    
	if (n.has_matrix) {

		float4x4 mat;
		for (int i = 0; i < 16; i++)
		{
			mat[i] = n.matrix[i];
		}
		result = math::to_transform(mat);
	}    
	if (n.has_translation) 
	{
		result.position = float3(n.translation[0], n.translation[1], n.translation[2]);
	}    
	if (n.has_rotation) 
	{
		result.rotation = quaternion(n.rotation[0], n.rotation[1], n.rotation[2], n.rotation[3]); 
	}  
	if (n.has_scale)
	{ 
		result.scale = float3(n.scale[0], n.scale[1], n.scale[2]); 
	}    
	return result;
}

int get_node_index(cgltf_node* target, cgltf_node* all_nodes, size_t num_nodes)
{
	if (target == 0) 
	{
		return -1;
	}
	for (size_t i = 0; i < num_nodes; ++i)
	{
		if (target == &all_nodes[i]) 
		{
			return(int)i;
		}
	}   
	return -1;
}

void get_scalar_values(std::vector<float>& out, size_t components, const cgltf_accessor& in_accessor)
{
	out.resize(in_accessor.count * components);    
	for (cgltf_size i = 0; i < in_accessor.count; ++i) 
	{
		cgltf_accessor_read_float(&in_accessor, i, &out[i * components], components);
	}
}

void track_from_channel(float3_track& out, const cgltf_animation_channel& channel)
{
	cgltf_animation_sampler& sampler = *channel.sampler;
	interpolation_type interpolation = interpolation_type::constant;

	if (sampler.interpolation == cgltf_interpolation_type_linear)
	{
		interpolation = interpolation_type::linear;
	}
	else if (sampler.interpolation == cgltf_interpolation_type_cubic_spline)
	{
		interpolation = interpolation_type::cubic;
	}

	out.type = interpolation;

	std::vector<float>time;   
	std::vector<float>val;

	get_scalar_values(time, 1, *sampler.input);
	get_scalar_values(val, 3, *sampler.output); 

	size_t num_frames = sampler.input->count;
	size_t components = val.size() / time.size();
	out.resize(num_frames);
	for (size_t i = 0; i < num_frames; ++i)
	{
		int baseIndex = i * (int)components;       
		float3_frame &frame = out[i];
		int offset = 0;        
		frame.t = time[i];        
		for (int comp = 0; comp < 3; ++comp) 
		{
			frame.in[comp] = interpolation == interpolation_type::cubic ? val[baseIndex + offset++] : 0.0f;
		}        
		for (int comp = 0; comp < 3; ++comp) 
		{ 
			frame.value[comp] = val[baseIndex + offset++]; 
		}  
		for (int comp = 0; comp < 3; ++comp) 
		{ 
			frame.out[comp] = interpolation == interpolation_type::cubic ? val[baseIndex + offset++] : 0.0f;
		}
	}
}


void track_from_channel(quaternion_track& out, const cgltf_animation_channel& channel)
{
	cgltf_animation_sampler& sampler = *channel.sampler;
	interpolation_type interpolation = interpolation_type::constant;

	if (sampler.interpolation == cgltf_interpolation_type_linear)
	{
		interpolation = interpolation_type::linear;
	}
	else if (sampler.interpolation == cgltf_interpolation_type_cubic_spline)
	{
		interpolation = interpolation_type::cubic;
	}

	out.type = interpolation;

	std::vector<float>time;
	std::vector<float>val;

	get_scalar_values(time, 1, *sampler.input);
	get_scalar_values(val, 4, *sampler.output);

	size_t num_frames = sampler.input->count;
	size_t components = val.size() / time.size();
	out.resize(num_frames);
	for (size_t i = 0; i < num_frames; ++i)
	{
		int baseIndex = i * (int)components;
		quaternion_frame& frame = out[i];
		int offset = 0;
		frame.t = time[i];
		for (int comp = 0; comp < 4; ++comp)
		{
			frame.in[comp] = interpolation == interpolation_type::cubic ? val[baseIndex + offset++] : 0.0f;
		}
		for (int comp = 0; comp < 4; ++comp)
		{
			frame.value[comp] = val[baseIndex + offset++];
		}
		for (int comp = 0; comp < 4; ++comp)
		{
			frame.out[comp] = interpolation == interpolation_type::cubic ? val[baseIndex + offset++] : 0.0f;
		}
	}
}

void load_animation_clips(std::vector<clip>& clips, cgltf_data* data)
{
	size_t num_clips = data->animations_count;   
	size_t num_nodes = data->nodes_count;    
	clips.resize(num_clips);
	for (size_t i = 0; i < num_clips; ++i)
	{
		clips[i].name = data->animations[i].name;
		size_t num_channels = data->animations[i].channels_count;
		for (size_t j = 0; j < num_channels; ++j)
		{
			cgltf_animation_channel& channel = data->animations[i].channels[j];
			cgltf_node* target = channel.target_node;
			int node_id = get_node_index(target, data->nodes, num_nodes);

			if (channel.target_path == cgltf_animation_path_type_translation)
			{
				float3_track& track = clips[i][node_id].position;
				track_from_channel(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_scale)
			{
				float3_track& track = clips[i][node_id].scale;
				track_from_channel(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_rotation)
			{
				quaternion_track& track = clips[i][node_id].rotation;
				track_from_channel(track, channel);
			}
			clips[i].recalculate_duration();
		}
	}
}

std::vector<std::string> load_joint_names(cgltf_data* data)
{
	size_t bone_Count = (size_t)data->nodes_count;
	std::vector<std::string> result(bone_Count, "");   
	for (size_t i = 0; i < bone_Count; ++i)
	{
		cgltf_node* node = &(data->nodes[i]);

		if (node->name != nullptr)
		{
			result[i] = node->name;
		}
	}    
	return result;
}