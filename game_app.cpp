#include "game_app.h"
#include "voxels.h"
#include <WinUser.h>
#include <future>
#include "skinned_vertex.h"
#include "vertex.h"

using namespace std::chrono_literals;
bool game_app::running = false;

float noise(float3 position, int octaves, float frequency, float persistence) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += math::improved_noise::noise(position * frequency) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
	}
	return total / maxAmplitude;
}

void game_app::initialize()
{
	srand((unsigned int)time(nullptr));
	math::improved_noise::init();

	wm.create(1280, 720, L"test");
	render.create_vulkan_context("test", wm.window, int2{ wm.wr.right, wm.wr.bottom });
	resources.initialize(&render.device);

	anim_sys.initialize(&resources.clips, &resources.rigs);
	mesh bunny_mesh;
	mesh teapot_mesh;
	mesh cube_mesh;


	skinned_mesh goblin = resources.load_skinned_mesh("assets/woman.gltf");
	clip goblin_clip = resources.load_animation("assets/woman.gltf");


	auto mesh_load_future = std::async(std::launch::async, [this, &bunny_mesh, &teapot_mesh, &cube_mesh]() {
		bunny_mesh = resources.load_mesh("assets/hana.fbx");
		teapot_mesh = resources.load_mesh("assets/teapot.fbx");
		cube_mesh = resources.load_mesh("assets/cube.fbx");
	});
	uint32_t plastic_material;
	uint32_t rock_material;

	auto plastic_future = std::async(std::launch::async, [this, &plastic_material]() {
		plastic_material = resources.load_material("assets/textures/plastic_color.png", "assets/textures/plastic_normal.png", "assets/textures/plastic_r_m_ao.png");
	});

	auto rock_future = std::async(std::launch::async, [this, &rock_material]() {
		rock_material = resources.load_material("assets/textures/metal_color.png", "assets/textures/metal_normal.png", "assets/textures/metal_r_m_ao.png");
		});

	running = true;

	em::voxel_storage storage = em::voxel_storage(uint32_3(128, 128, 128));
	auto color_id = storage.add_color(uint32_4(0, 0, 0, 0));
	color_id = storage.add_color(uint32_4(255, 35, 128, 255));
	auto dirt = storage.add_color(uint32_4( 95, 46, 0, 255));

	locked_mouse = true;
	wm.set_cursor_locked(locked_mouse, { window_center });
	int world_size = 128;

	for (int i = 30; i < world_size - 30; i++)
	{
		for (int j = 10; j < 30; j++)
		{
			for (int k = 30; k < world_size - 30; k++)
			{
				float f = noise(float3((float)i, (float)j, (float)(rand() % 100)), 5, 3, 1);
				if (f > 10)
				{
					storage.set_voxel(uint32_3(i, j, k), dirt);
				}
			}
		}
	}


	for (int i = 5; i < world_size-5; i++)
	{
		for (int j = 1; j < 3; j++)
		{
			for (int k = 5; k < world_size -5; k++)
			{
				storage.set_voxel(uint32_3(i, j, k), dirt);
			}
		}
	}

	for (int i = 1; i < world_size - 1; i++)
	{
		for (int j = 7; j < 10; j++)
		{
			for (int k = 1; k < world_size - 1; k++)
			{
				storage.set_voxel(uint32_3(i, j, k), dirt);
			}
		}
	}

	archetype<position, directional_light> light_components;

	archetype<position, point_light> p_light_components;
	archetype<position, renderable> renderable_components;
	archetype<position, renderable, animation> animation_components;
	auto light = ecs.create_entity(light_components.descriptor(), 5);
	auto& lightcomp = ecs.get_component<directional_light>(light);

	lightcomp.color = float4(2.5f, 2.5f, 2.5f, 1.0f);
	lightcomp.direction = float4(0, -0.8f, 0.3f, 1.0f);

	//for (int i = 0; i < 5; i++)
	//{
	//	auto p_light = ecs.create_entity(p_light_components.descriptor(), 5);
	//	auto& p_light_comp = ecs.get_component<point_light>(p_light);
	//	p_light_comp.color = float4(1.0f, 0.3f, 0.6f, 1.0f);

	//	auto& p_light_pos = ecs.get_component<position>(p_light);
	//	p_light_pos.x = i * 1.5f;
	//	p_light_pos.y = 3; 
	//	p_light_pos.z = 0;
	//}
	


	em::polygenerator::generate(storage, resources);
	mesh_load_future.get();
	plastic_future.get();
	rock_future.get();

	em::descriptor_set_settings static_mesh_descriptor;
	static_mesh_descriptor.materials = resources.materials;
	em::descriptor_set_settings skinned_mesh_descriptor;
	skinned_mesh_descriptor.materials = resources.materials;

	render.create_swapchain();
	uint32_t skinned_mesh_desc_index = render.create_descriptor_set(skinned_mesh_descriptor);
	uint32_t static_mesh_desc_index = render.create_descriptor_set(static_mesh_descriptor);
	uint32_t flag_static_mesh_desc_index = render.create_descriptor_set(static_mesh_descriptor);


	em::graphics_pipeline_settings static_mesh_pipeline;
	em::graphics_pipeline_settings skinned_mesh_pipeline;
	em::graphics_pipeline_settings flat_static_mesh_pipeline;

	get_attrib_description(static_mesh_pipeline.attribute_desc);
	static_mesh_pipeline.cullmode = VK_CULL_MODE_NONE;
	static_mesh_pipeline.binding_desc = get_binding_desc();
	static_mesh_pipeline.sample_count = VK_SAMPLE_COUNT_1_BIT;
	static_mesh_pipeline.desc_layout = static_mesh_desc_index;
	static_mesh_pipeline.primitives = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	static_mesh_pipeline.winding = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	static_mesh_pipeline.shaders.push_back(resources.load_shader("shader/lit.vert.spv", em::shader_type::VK_SHADER_STAGE_VERTEX_BIT));
	static_mesh_pipeline.shaders.push_back(resources.load_shader("shader/lit.frag.spv", em::shader_type::VK_SHADER_STAGE_FRAGMENT_BIT));

	uint32_t static_mesh_pipeline_index = render.create_pipeline(static_mesh_pipeline);


	get_attrib_description(flat_static_mesh_pipeline.attribute_desc);
	flat_static_mesh_pipeline.cullmode = VK_CULL_MODE_NONE;
	flat_static_mesh_pipeline.binding_desc = get_binding_desc();
	flat_static_mesh_pipeline.sample_count = VK_SAMPLE_COUNT_1_BIT;
	flat_static_mesh_pipeline.desc_layout = flag_static_mesh_desc_index;
	flat_static_mesh_pipeline.primitives = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	flat_static_mesh_pipeline.winding = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	flat_static_mesh_pipeline.shaders.push_back(resources.load_shader("shader/lit.vert.spv", em::shader_type::VK_SHADER_STAGE_VERTEX_BIT));
	flat_static_mesh_pipeline.shaders.push_back(resources.load_shader("shader/lit-flat-shaded.frag.spv", em::shader_type::VK_SHADER_STAGE_FRAGMENT_BIT));

	uint32_t flat_static_mesh_pipeline_index = render.create_pipeline(flat_static_mesh_pipeline);


	skinned_vertex_get_attrib_description(skinned_mesh_pipeline.attribute_desc);
	skinned_mesh_pipeline.cullmode = VK_CULL_MODE_NONE;
	skinned_mesh_pipeline.binding_desc = skinned_vertex_get_binding_desc();
	skinned_mesh_pipeline.sample_count = VK_SAMPLE_COUNT_1_BIT;
	skinned_mesh_pipeline.desc_layout = skinned_mesh_desc_index;
	skinned_mesh_pipeline.primitives = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	skinned_mesh_pipeline.winding = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	skinned_mesh_pipeline.shaders.push_back(resources.load_shader("shader/lit_skinned.vert.spv", em::shader_type::VK_SHADER_STAGE_VERTEX_BIT));
	skinned_mesh_pipeline.shaders.push_back(resources.load_shader("shader/lit.frag.spv", em::shader_type::VK_SHADER_STAGE_FRAGMENT_BIT));

	uint32_t skinned_mesh_pipeline_index = render.create_pipeline(skinned_mesh_pipeline);

	//auto voxel_mesh = resources.load_mesh("voxels");
	//auto e =  ecs.create_entity(renderable_components.descriptor(),  50);
	//auto& rend = ecs.get_component<renderable>(e);
	//rend.vbo = voxel_mesh.vbo.buffer;
	//rend.vert_count = voxel_mesh.vertex_count;
	//rend.material = rock_material;
	//rend.vertex_stride = sizeof(vertex);b
	//rend.pipeline = flat_static_mesh_pipeline_index;
	//rend.desc = flag_static_mesh_desc_index;

	auto e = ecs.create_entity(animation_components.descriptor(), 1);
	auto& rend5 = ecs.get_component<renderable>(e);
	rend5.vbo = goblin._mesh.vbo.buffer;
	rend5.vert_count = goblin._mesh.vertex_count;
	rend5.material = rock_material;
	rend5.vertex_stride = sizeof(skinned_vertex);
	rend5.pipeline = skinned_mesh_pipeline_index;
	rend5.desc = skinned_mesh_desc_index;

	auto& anim = ecs.get_component<animation>(e);
	anim.animation_clip = 0;
	anim.rig = 0;

	auto& p5 = ecs.get_component<position>(e);
	p5.x = 0;
	p5.z = 0;
	p5.y = 0;


	//for (int i = 0; i < 10; i++)
	//{
	//	auto e2 = ecs.create_entity(renderable_components.descriptor());
	//	auto& rend3 = ecs.get_component<renderable>(e2);
	//	rend3.vbo = bunny_mesh.vbo.buffer;
	//	rend3.vert_count = bunny_mesh.vertex_count;
	//	rend3.material = rock_material;
	//	rend3.vertex_stride = sizeof(vertex);
	//	rend3.pipeline = static_mesh_pipeline_index;
	//	rend3.desc = static_mesh_desc_index;
	//	auto& pos2 = ecs.get_component<position>(e2);
	//	pos2.x = (float)(i / 5);
	//	pos2.y = (float)(i % 5);
	//	pos2.z = (float)5;
	//}

	//for (int i = 0; i < 10; i++)
	//{
	//	auto e2 = ecs.create_entity(renderable_components.descriptor());
	//	auto& rend3 = ecs.get_component<renderable>(e2);
	//	rend3.vbo = teapot_mesh.vbo.buffer;
	//	rend3.vert_count = teapot_mesh.vertex_count;
	//	rend3.material = rock_material;
	//	rend3.pipeline = static_mesh_pipeline_index;
	//	rend3.desc = static_mesh_desc_index;
	//	rend3.vertex_stride = sizeof(vertex);
	//	auto& pos2 = ecs.get_component<position>(e2);
	//	pos2.x = (float)(i / 5) * 10.0f;
	//	pos2.y = (float)(i % 5) * 10.0f;
	//	pos2.z = (float)10;
	//}

	//for (int i = 0; i < 10; i++)
	//{
	//	auto cube = ecs.create_entity(renderable_components.descriptor());
	//	auto& rend3 = ecs.get_component<renderable>(cube);
	//	rend3.vbo = cube_mesh.vbo.buffer;
	//	rend3.vert_count = cube_mesh.vertex_count;
	//	rend3.material = rock_material;
	//	rend3.vertex_stride = sizeof(vertex);
	//	rend3.pipeline = static_mesh_pipeline_index;
	//	rend3.desc = static_mesh_desc_index;

	//	auto& pos2 = ecs.get_component<position>(cube);
	//	pos2.x = 3 + (i / 5) * 0.25f;
	//	pos2.y = -3 + (i % 5) * 0.25f;
	//	pos2.z = (float)i;
	//}


	window_size = int2(wm.wr.right, wm.wr.bottom);
	window_center = window_size / 2;





	light_sys.initialize(&ecs);
	camera_sys.initialize(window_size);
	storage.dispose();
	base_time = clock::now();

	target_time = std::chrono::nanoseconds(16ms);
	lag = std::chrono::nanoseconds(0ms);
}

void game_app::release()
{
	wm.release();
}

void game_app::start()
{
	int fps_counter = 0;
	nano_seconds fps_timer = 0ns;
	std::vector<float4x4> poses;
	while (running)
	{
		wm.process_events();
		window_size = int2(wm.wr.right, wm.wr.bottom);
		window_center = window_size / 2;
		auto current_time = clock::now();

		lag += current_time - base_time;
		fps_timer += current_time - base_time;;
		base_time = current_time;
		if (lag > 0ms)
		{
			auto dt = std::min(lag, target_time);
			float dt_float = (float)(dt.count() / 100000000.0f);
			update(dt_float);
			anim_sys.update(&ecs, dt_float, poses);
			//std::cout << dt_float << std::endl;
			lag -= dt;
		}


		if (fps_timer >= 1s)
		{
			std::cout << fps_counter << std::endl;
			fps_counter = 0;
			fps_timer = 0s;
		}
		fps_counter++;
		render.render(window_size, light_sys.lbo, camera_sys.cam_data, render_sys.batches, poses);
	}
}

void game_app::update(float dt)
{
	if (wm.input_manager.key(0x1B))
	{
		running = false;
	}

	if (wm.input_manager.key(0x39))
	{
		locked_mouse = !locked_mouse;
		wm.set_cursor_locked(locked_mouse, { window_center });
	}

	
	light_sys.update(&ecs, dt);
	camera_sys.fps_camera_update(dt, wm.input_manager, window_center);
	render_sys.gather_renderables(&render, &ecs, camera_sys.vp);
}

void game_app::dispose()
{
	ecs.dispose();
}
