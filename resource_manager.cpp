#include "resource_manager.h"
#include "ofbx.h"
#include "vulkan_utils.h"
#include <future>
#include "gltf_loader.h"
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

clip resource_manager::load_animation(const char* file_path)
{
	auto res = animations.find(file_path);
	if (res != animations.end())
	{
		return res->second;
	}
	std::vector<clip> loaded_clips;

	cgltf_data* data = load_gltf_file(file_path);
	load_animation_clips(loaded_clips, data);
	
	std::string name = ""; 
	for (int i = 0; i < loaded_clips.size(); i++)
	{
		name = loaded_clips[i].name;
		animations[name.c_str()] = loaded_clips[i];
		clips.push_back(loaded_clips[i]);
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
	std::vector<skinned_vertex> indexed_vertices;
	std::vector<skinned_mesh> meshes;
	cgltf_data* data = load_gltf_file(file_path);

	cgltf_node* nodes = data->nodes;    
	size_t node_count = data->nodes_count;    

	for (size_t i = 0; i < node_count; ++i)
	{
		cgltf_node* node = &nodes[i];       
		if (node->mesh == 0 || node->skin == 0) { continue; }
		int num_primitives = node->mesh->primitives_count;       
		for (int j = 0; j < num_primitives; ++j)
		{
			cgltf_primitive* primitive = &node->mesh->primitives[j];           
			unsigned int ac = primitive->attributes_count;          
			for (unsigned int k = 0; k < ac; ++k) 
			{
				cgltf_attribute* attribute = &primitive -> attributes[k];                
				mesh_from_attribute(vertices, *attribute, node->skin, nodes, node_count);
			}
			if (primitive->indices != 0) 
			{
				int ic = primitive->indices->count;
				/*
					std::vector<unsigned int>& indices = mesh.GetIndices();
					indices.resize(ic);
				*/
				indexed_vertices.resize(ic);
				for (unsigned int k = 0; k < ic; ++k)
				{ 
					indexed_vertices.push_back(vertices[cgltf_accessor_read_index(primitive->indices, k)]);
				}
			}
			struct mesh mesh;
			device->load_mesh(indexed_vertices, mesh);
			skinned_mesh sm;
			sm._mesh = mesh;
			meshes.push_back(sm);
			indexed_vertices.clear();
			vertices.clear();
		}
	}   

	if (meshes.size() > 0)
	{
		meshes[0]._rig = load_skeleton(data);
		skinned_meshes[file_path] = meshes[0];
		rigs.push_back(meshes[0]._rig);
		return skinned_meshes[file_path];
	}
	return skinned_mesh();
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
