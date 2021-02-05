#include "game_app.h"
#include "voxels.h"
#include <WinUser.h>
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

	mesh bunny_mesh = resources.load_mesh("assets/bunny.fbx");
	mesh teapot_mesh = resources.load_mesh("assets/teapot.fbx");
	mesh cube_mesh = resources.load_mesh("assets/cube.fbx");
	
	//todo: normal maps loading
	//todo: texture map loading optimizations
	//todo: async loading
	//todo: multi threaded loading
	uint32_t plastic_material = resources.load_material("assets/textures/plastic_color.png", "assets/textures/plastic_normal.png", "assets/textures/plastic_r_m_ao.png");
	uint32_t rock_material = resources.load_material("assets/textures/metal_color.png", "assets/textures/metal_normal.png", "assets/textures/metal_r_m_ao.png");

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
	archetype<position, renderable> renderable_components;
	auto e = ecs.create_entity(light_components.descriptor(), 5);
	auto& lightcomp=  ecs.get_component<directional_light>(e);

	lightcomp.color = float4(2.5f, 2.5f, 2.5f, 1.0f);
	lightcomp.direction = float4(0, -0.8f, 0.3f, 1.0f);

	em::polygenerator::generate(storage, resources);


	auto voxel_mesh = resources.load_mesh("voxels");
	e =  ecs.create_entity(renderable_components.descriptor(),  400);
	auto& rend = ecs.get_component<renderable>(e);
	rend.vbo = voxel_mesh.vbo.buffer;
	rend.vert_count = voxel_mesh.vertex_count;
	rend.material = plastic_material;

	for (int i = 0; i < 100; i++)
	{
		auto e2 = ecs.create_entity(renderable_components.descriptor());
		auto& rend3 = ecs.get_component<renderable>(e2);
		rend3.vbo = bunny_mesh.vbo.buffer;
		rend3.vert_count = bunny_mesh.vertex_count;

		auto& pos2 = ecs.get_component<position>(e2);
		pos2.x = i / 5;
		pos2.y = i % 5;
		pos2.z = 5;
	}

	for (int i = 0; i < 100; i++)
	{
		auto e2 = ecs.create_entity(renderable_components.descriptor());
		auto& rend3 = ecs.get_component<renderable>(e2);
		rend3.vbo = teapot_mesh.vbo.buffer;
		rend3.vert_count = teapot_mesh.vertex_count;
		rend3.material = rock_material;

		auto& pos2 = ecs.get_component<position>(e2);
		pos2.x = (i / 5) * 10;
		pos2.y = (i % 5) * 10;
		pos2.z = 10;
	}

	for (int i = 0; i < 128; i++)
	{
		auto cube = ecs.create_entity(renderable_components.descriptor());
		auto& rend3 = ecs.get_component<renderable>(cube);
		rend3.vbo = cube_mesh.vbo.buffer;
		rend3.vert_count = cube_mesh.vertex_count;

		auto& pos2 = ecs.get_component<position>(cube);
		pos2.x = 3 + (i / 5) * 0.25f;
		pos2.y = -3 + (i % 5) * 0.25f;
		pos2.z = i;
	}


	window_size = int2(wm.wr.right, wm.wr.bottom);
	window_center = window_size / 2;

	resources.load_shader("shader/lit.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	resources.load_shader("shader/lit.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	render.create_pipeline(&resources.shaders, &resources.materials);
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
		render.render(window_size, light_sys.lbo, camera_sys.cam_data, render_sys.batches);
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
