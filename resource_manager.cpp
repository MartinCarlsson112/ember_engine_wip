#include "resource_manager.h"
#include "ofbx.h"
#include "vulkan_utils.h"

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

uint32_t resource_manager::load_material(const char* albedo_path, const char* normal_path, const char* m_r_ao_path)
{
	uint32_t index = materials.size() / 3;
	materials.push_back(load_texture(albedo_path));
	materials.push_back(load_texture(normal_path));
	materials.push_back(load_texture(m_r_ao_path));
	return index;
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
	if (shader_path_to_index.find(file_path) != shader_path_to_index.end())
	{
		return shaders[shader_path_to_index[file_path]];
	}
	shader_path_to_index[file_path] = shaders.size();
	shaders.push_back(load_shader_impl(*device, file_path, type));
	return shaders[shaders.size() - 1];
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
