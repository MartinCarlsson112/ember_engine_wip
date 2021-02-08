#include "resource_manager.h"
#include "ofbx.h"
#include "vulkan_utils.h"
#include <future>
void resource_manager::initialize(em::device* device)
{
    this->device = device;
}

mesh resource_manager::load_mesh(const char* name, const std::vector<vertex>& verts)
{
	auto res = meshes.find(name);
	if (res != meshes.end())
	{
		return res->second;
	}
	meshes.insert(std::make_pair(name, load_mesh(verts)));
	return meshes[name];
}

mesh resource_manager::load_mesh(const char* file_path)
{
    auto res = meshes.find(file_path);
    if (res != meshes.end())
    {
        return res->second;
    }

	std::vector<vertex> vertices;

	FILE* fp = fopen(file_path, "rb");
	if (!fp)
	{
		return mesh();
	}

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);

	ofbx::IScene* scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);

	for (int i = 0; i < scene->getMeshCount(); i++)
	{
		const ofbx::Mesh* mesh = scene->getMesh(i);
		const ofbx::Geometry* geo = mesh->getGeometry();

		vertices.resize(geo->getVertexCount());

		const ofbx::Vec3* vert_buffer = geo->getVertices();
		const ofbx::Vec2* uv_buffer = geo->getUVs();
		const ofbx::Vec3* normal_buffer = geo->getNormals();

		for (int j = 0; j < geo->getVertexCount(); j++)
		{
			vertices[j].position = float4{ (float)vert_buffer[j].x, (float)vert_buffer[j].y, (float)vert_buffer[j].z, 1};
			vertices[j].normal = float4{ (float)normal_buffer[j].x, (float)normal_buffer[j].y, (float)normal_buffer[j].z, 1};
			if (uv_buffer != nullptr)
			{
				vertices[j].uv = float2{ (float)uv_buffer[j].x, 1.0f - (float)uv_buffer[j].y };
			}
		}
	}
	scene->destroy();
	meshes.insert(std::make_pair(file_path, load_mesh(vertices)));
    return 	meshes[file_path];
}

mesh resource_manager::load_mesh(const std::vector<vertex>& verts)
{
	struct mesh mesh;
	device->load_mesh(verts, mesh);
	return mesh;
}


mesh resource_manager::load_mesh(const std::vector<skinned_vertex>& verts)
{
	struct mesh mesh;
	device->load_mesh(verts, mesh);
	return mesh;
}

struct anim_info
{
	float time_start;
	float time_end;
	float duration;
	int frames_count;
	float sampling_period;
};

clip resource_manager::load_animation(const char* file_path, const rig& skeleton)
{
	auto res = animations.find(file_path);
	if (res != animations.end())
	{
		return res->second;
	}

	std::vector<vertex> vertices;

	FILE* fp = fopen(file_path, "rb");
	if (!fp)
	{
		return clip();
	}

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);

	ofbx::IScene* scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::IGNORE_GEOMETRY);

	if (scene)
	{
		int animation_count = scene->getAnimationStackCount();
		auto scene_frame_rate = scene->getSceneFrameRate();


		for (int i = 0; i < animation_count; i++)
		{
			clips.push_back(clip());
			clip& anim_clip = clips[clips.size()-1];
			auto anim_stack = scene->getAnimationStack(i);
			auto take_info = scene->getTakeInfo(anim_stack->name);
			auto layer = anim_stack->getLayer(0);
			anim_clip.name = anim_stack->name;

			auto node_counter = 0;
			const float local_duration = (float)(take_info->local_time_to - take_info->local_time_from);
			int duration = (int)(local_duration * scene_frame_rate + 0.5f);
			anim_info info;
			info.time_start = (float)take_info->local_time_from;
			info.time_end = (float)take_info->local_time_to;
			info.duration = local_duration;
			info.frames_count = duration;
			info.sampling_period = 1.0f / scene_frame_rate;

			anim_clip.start_time = info.time_start;
			anim_clip.end_time = info.time_end;

			auto node = layer->getCurveNode(node_counter);

			while (node != nullptr && node->getBone() != nullptr)
			{
				auto bone = node->getBone();

				auto bone_id = -1;
				for (int v = 0; v < skeleton.joint_names.size(); v++)
				{
					if (bone->name == skeleton.joint_names[v])
					{
						bone_id = v;
					}
				}
				auto translation = layer->getCurveNode(*bone, "Lcl Translation");
				auto rotation = layer->getCurveNode(*bone, "Lcl Rotation");
				auto scale = layer->getCurveNode(*bone, "Lcl Scaling");

				if (bone_id != -1)
				{
					anim_clip.tracks.push_back(transform_track());
					anim_clip.tracks[anim_clip.tracks.size() - 1].bone_index = bone_id;
					anim_clip.tracks[anim_clip.tracks.size() - 1].position.resize(info.frames_count);
					anim_clip.tracks[anim_clip.tracks.size() - 1].rotation.resize(info.frames_count);
					anim_clip.tracks[anim_clip.tracks.size() - 1].scale.resize(info.frames_count);
					for (int j = 0; j < info.frames_count; j++)
					{
						const double time = info.time_start + ((double)j / info.frames_count) * info.duration;
						ofbx::Vec3 t = bone->getLocalTranslation();
						ofbx::Vec3 r = bone->getLocalRotation();
						ofbx::Vec3 s = bone->getLocalScaling();

						if (translation)
						{
							ofbx::Vec3 local_t = translation->getNodeLocalTransform(time);
							auto el_t = bone->evalLocal(local_t, r, s);
							anim_clip.tracks[anim_clip.tracks.size() - 1].position[j].t = (float)time;
							anim_clip.tracks[anim_clip.size() - 1].position[j].value = float3((float)el_t.m[12], (float)el_t.m[13], (float)el_t.m[14]);
						}

						if (rotation)
						{
							ofbx::Vec3 local_r = rotation->getNodeLocalTransform(time);
							auto el_r = bone->evalLocal(t, local_r, s);
							anim_clip.tracks[anim_clip.tracks.size() - 1].rotation[j].t = (float)time;

							float4x4 rotation_mat;
							for (int i = 0; i < 16; i++)
							{
								rotation_mat[i] = (float)el_r.m[i];
							}
							quaternion rot = math::from_float4x4(rotation_mat);
							anim_clip.tracks[anim_clip.size() - 1].rotation[j].value = rot;
						}

						if (scale)
						{
							ofbx::Vec3 local_s = scale->getNodeLocalTransform(time);
							auto el_s = bone->evalLocal(t, r, local_s);
							anim_clip.tracks[anim_clip.tracks.size() - 1].scale[j].t = (float)time;

							float3 s;
							s.x = math::length(float3((float)el_s.m[0], (float)el_s.m[1], (float)el_s.m[2]));
							s.y = math::length(float3((float)el_s.m[4], (float)el_s.m[5], (float)el_s.m[6]));
							s.z = math::length(float3((float)el_s.m[8], (float)el_s.m[9], (float)el_s.m[10]));
							anim_clip.tracks[anim_clip.size() - 1].scale[j].value = s;
						}
						else
						{
							anim_clip.tracks[anim_clip.size() - 1].scale[j].value = float3(1.0f, 1.0f, 1.0f);
						}
					}
				}
				if (translation)
				{
					node_counter++;
				}
				if (rotation)
				{
					node_counter++;
				}
				if (scale)
				{
					node_counter++;
				}
				node = layer->getCurveNode(node_counter);
			}	
		}
	}
	std::string name = ""; 
	for (int i = 0; i < clips.size(); i++)
	{
		name = clips[i].name;
		animations[name.c_str()] = clips[i];
	}
	return animations[name.c_str()];
}

uint32_t resource_manager::load_material(const char* albedo_path, const char* normal_path, const char* m_r_ao_path)
{
	uint32_t index = (uint32_t)materials.size() / 3;

	em::texture color, normal, mrao;

	auto color_future = std::async(std::launch::async, [this, &color, &albedo_path]() {
		color = load_texture(albedo_path);
	});

	auto normal_future = std::async(std::launch::async, [this, &normal, &normal_path]() {
		normal = load_texture(normal_path);
	});
	auto mrao_future = std::async(std::launch::async, [this, &mrao, &m_r_ao_path]() {
		mrao = load_texture(m_r_ao_path);
	});

	color_future.get();
	normal_future.get();
	mrao_future.get();

	materials.push_back(color);
	materials.push_back(normal);
	materials.push_back(mrao);
	return index;
}

skinned_mesh resource_manager::load_skinned_mesh(const char* file_path)
{
	auto res = skinned_meshes.find(file_path);
	if (res != skinned_meshes.end())
	{
		return res->second;
	}

	std::vector<skinned_vertex> vertices;

	FILE* fp = fopen(file_path, "rb");
	if (!fp)
	{
		return skinned_mesh();
	}

	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);

	ofbx::IScene* scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
	std::vector<skinned_mesh> _meshes;
	float4x4 bind_pose;
	float4x4 rest_pose;
	float4x4 inverse_bind_pose;
	for (int i = 0; i < scene->getMeshCount(); i++)
	{
		const ofbx::Mesh* mesh = scene->getMesh(i);
		const ofbx::Geometry* geo = mesh->getGeometry();
		vertices.resize(geo->getVertexCount());

		const ofbx::Skin* skin = geo->getSkin();
		const int clusters = skin->getClusterCount();
		_meshes.push_back(skinned_mesh());

		for (int j = 0; j < clusters; j++)
		{
			const ofbx::Cluster* cluster = skin->getCluster(j);
			auto indices_count = cluster->getIndicesCount();

			if (indices_count <= 0)
			{
				continue;
			}
			auto indices = cluster->getIndices();
			auto weights = cluster->getWeights();
			for (int k = 0; k < indices_count; k++)
			{
				if (vertices[indices[k]].bones.x == -1)
				{
					vertices[indices[k]].bones.x = j;
					vertices[indices[k]].weights.x = (float)weights[k];
				}
				else if (vertices[indices[k]].bones.y == -1)
				{
					vertices[indices[k]].bones.y = j;
					vertices[indices[k]].weights.y = (float)weights[k];
				}				
				else if (vertices[indices[k]].bones.z == -1)
				{
					vertices[indices[k]].bones.z = j;
					vertices[indices[k]].weights.z = (float)weights[k];
				}				
				else if (vertices[indices[k]].bones.w == -1)
				{
					vertices[indices[k]].bones.w = j;
					vertices[indices[k]].weights.w = (float)weights[k];
				}
			}

			auto parent = cluster->getLink()->resolveObjectLinkReverse(ofbx::Object::Type::LIMB_NODE);
			int parent_index = -1;
			if (parent != nullptr)
			{
				for (int v = 0; v < clusters; v++)
				{
					if (parent == skin->getCluster(v)->getLink())
					{
						parent_index = v;
					}
				}
			}

			//add joint to skeleton
			//TODO: first place to check, idk how this works in ofbx
			auto bind_matrix = cluster->getTransformLinkMatrix();
			auto rest_matrix = cluster->getTransformMatrix();
			//convert ofbx matrices to float4x4 -todo: i dont remember ofbx row/column order
			for (int i = 0; i < 16; i++)
			{
				bind_pose[i] = (float)bind_matrix.m[i];
				rest_pose[i] = (float)rest_matrix.m[i];
			}
			//todo: third place to check, idk if this to_transform is correct
			transform rest_transform = math::to_transform(rest_pose);
			transform bind_transform = math::to_transform(bind_pose);
			math::inverse_matrix(bind_pose, inverse_bind_pose);

			_meshes[i]._rig.add_joint(bind_transform, rest_transform, parent_index, cluster->getLink()->name, inverse_bind_pose);
		}
		const ofbx::Vec3* vert_buffer = geo->getVertices();
		const ofbx::Vec2* uv_buffer = geo->getUVs();
		const ofbx::Vec3* normal_buffer = geo->getNormals();

		for (int j = 0; j < geo->getVertexCount(); j++)
		{
			vertices[j].position = float4{ (float)vert_buffer[j].x * 0.01f, (float)vert_buffer[j].y * 0.01f, (float)vert_buffer[j].z * 0.01f, 1 };
			vertices[j].normal = float3{ (float)normal_buffer[j].x, (float)normal_buffer[j].y, (float)normal_buffer[j].z };
			if (uv_buffer != nullptr)
			{
				vertices[j].uvs = float2{ (float)uv_buffer[j].x, 1.0f - (float)uv_buffer[j].y };
			}
		}

		_meshes[i]._mesh = load_mesh(vertices);
		vertices.clear();
	}
	scene->destroy();
	skinned_meshes[file_path] = _meshes[1];
	rigs.push_back(_meshes[1]._rig);
	return 	skinned_meshes[file_path];

}



em::texture resource_manager::load_texture(const char* file_path)
{
    auto res = textures.find(file_path);
    if (res != textures.end())
    {
        return res->second;
    }
	em::texture tex = em::texture();
	tex.load(*device, file_path);
	textures[file_path] = tex;
    return textures[file_path];
}

em::shader resource_manager::load_shader(const char* file_path, em::shader_type type)
{
	if (shaders.find(file_path) != shaders.end())
	{
		return shaders[file_path];
	}
	shaders[file_path] = load_shader_impl(*device, file_path, type);
	return shaders[file_path];
}


void resource_manager::clear()
{
    for (auto& mesh : meshes)
    {
		auto v = mesh.second;
		device->destroy_mesh(v);
    }
    meshes.clear();

    for (auto& tex : textures)
    {
		auto t = tex.second;
		device->destroy_image(t.io);
    }
    textures.clear();
}

void resource_manager::unload_mesh(const char* filePath)
{

}

void resource_manager::unload_texture(const char* filePath)
{

}
