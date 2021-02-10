#pragma once
#include <unordered_map>
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "skinned_mesh.h"
#include "skinned_vertex.h"
#include "animation.h"
#include "device.h"
class resource_manager
{
public:
	void initialize(em::device* device);

	mesh load_mesh(const char* filePath);
	mesh load_mesh(const char* name, const std::vector<vertex>& verts);
	mesh load_mesh(const std::vector<vertex>& verts);

	uint32_t load_material(const char* albedo_path, const char* normal_path, const char* m_r_ao_path);

	skinned_mesh load_skinned_mesh(const char* file_path);
	mesh load_mesh(const std::vector<skinned_vertex>& verts);

	clip load_animation(const char* file_path);

	em::texture load_texture(const char* file_Path);
	em::shader load_shader(const char* file_path, em::shader_type);

	std::unordered_map<const char*, mesh> meshes;
	std::unordered_map<const char*, em::texture> textures;
	std::unordered_map<const char*, em::shader> shaders;
	std::unordered_map<const char*, uint32_t> material_to_index;
	std::unordered_map<const char*, skinned_mesh> skinned_meshes;
	std::unordered_map<const char*, clip> animations;
	std::vector<em::texture> materials;

	std::vector<rig> rigs;
	std::vector<clip> clips;

	void clear();

	void unload_mesh(const char* filePath);
	void unload_texture(const char* filePath);
	em::device* device;

private:

};