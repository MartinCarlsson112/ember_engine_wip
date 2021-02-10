#pragma once
#include "cgltf.h"
#include "animation.h"
#include <vector>
#include "mmath.h"
#include "skinned_vertex.h"


cgltf_data* load_gltf_file(const char* file_path);

void free_gltf_file(cgltf_data* handle);

void mesh_from_attribute(std::vector<skinned_vertex>& vertices, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, size_t node_count);

rig load_skeleton(cgltf_data* data);
pose load_rest_pose(cgltf_data* data);
pose load_bind_pose(cgltf_data* data);
std::vector<std::string> load_joint_names(cgltf_data* data);

transform get_local_transform(cgltf_node& n);
int get_node_index(cgltf_node* target, cgltf_node* all_nodes, size_t num_nodes);
void get_scalar_values(std::vector<float>& out, size_t components, const cgltf_accessor& in_accessor);
void track_from_channel(float3_track& out, const cgltf_animation_channel& channel);
void track_from_channel(quaternion_track& out, const cgltf_animation_channel& channel);
void load_animation_clips(std::vector<clip>& clips, cgltf_data* data);

