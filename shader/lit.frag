#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 2) in vec3 view_pos;
layout(location = 3) in vec3 frag_pos;

layout(binding = 2) uniform sampler2D shadow_map;

#define MAX_TEXTURES 32
layout(binding = 4) uniform sampler2D material_textures[MAX_TEXTURES];

layout(location = 0) out vec4 out_color;
struct directional_light
{
	vec4 position;
	vec4 diffuse;
	vec4 direction;
};

struct point_light
{
	vec4 position;
	vec4 diffuse;
};


#define MAX_DIR_LIGHT 5
#define MAX_POINT_LIGHTS 5
layout(set = 0, binding = 1) uniform light_data
{
	directional_light[MAX_DIR_LIGHT] dir_lights;
	point_light[MAX_POINT_LIGHTS] point_lights;
};


layout(push_constant) uniform material_index {
    layout(offset = 64) uint index;
};

struct material
{
	vec3 albedo; 
	float metallic;
	float roughness;
	float ao;
};

const float PI = 3.14159265359;

float dist_ggx(vec3 normal, vec3 h, float roughness)
{
	float roct = roughness * roughness * roughness * roughness;
	float ndoth = max(dot(normal, h), 0.0);
	ndoth *= ndoth;
	float denom = ndoth * (roct -1.0) + 1.0;
	denom = PI * denom * denom;
	return roct / denom;
}

float geo_schlick_ggx(float ndotv, float roughness)
{
	float r = (roughness +1);
	float k = (r*r) / 8.0;
	float denom = ndotv * (1.0-k) + k;
	return ndotv/denom;
}

float geo_smith(vec3 normal, vec3 view, vec3 light_dir, float roughness)
{
	float ndotv = max(dot(normal, view), 0.0);
	float ndotl = max(dot(normal, light_dir), 0.0);
	float ggx1 = geo_schlick_ggx(ndotl, roughness);
	float ggx2 = geo_schlick_ggx(ndotv, roughness);
	return ggx1 * ggx2;
}



vec3 fresnel_schlick_roughness(float cos_theta, vec3 f0, float roughness)
{
    float om_cos_theta = 1.0 - cos_theta;
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * (om_cos_theta * om_cos_theta * om_cos_theta * om_cos_theta * om_cos_theta);
}


vec3 per_light_calc(vec3 light_dir, vec3 diffuse, material mat, vec3 normal, vec3 view_dir, vec3 f0)
{
	vec3 H = normalize( view_dir + light_dir);
	vec3 radiance = mat.albedo;

	float NDF  = dist_ggx(normal, H, mat.roughness);
	float G = geo_smith(normal, view_dir, light_dir, mat.roughness);
	vec3 fresnel = fresnel_schlick_roughness(max(dot(H, view_dir), 0.0), f0, mat.roughness);


	vec3 kd = vec3(1.0) - fresnel;

	vec3 spec = (NDF * G * fresnel) / 
		(4 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0) + 0.001);
	
	float ndotl = max(dot(normal, light_dir), 0.0);

	return (kd * diffuse.rgb / PI + spec) * radiance * ndotl;
}


void main() {
	vec3 flat_normal = normalize(cross(dFdx(vec3(frag_pos)), dFdy(vec3(frag_pos))));
	material mat; //temporary material
	uint material_ind = index * 3;

	mat.albedo = vec3(texture(material_textures[material_ind], uv));
	
	//normal map
	vec3 rmao = vec3(texture(material_textures[material_ind+2], uv));

	mat.roughness = rmao.r;
	mat.metallic = rmao.g;
	mat.ao = rmao.b;

	ivec2 screen_size = textureSize(shadow_map, 0); 

	vec2 shadow_uv = gl_FragCoord.xy / screen_size;
	vec3 shadow = vec3(texture(shadow_map, shadow_uv));
	if(shadow.x > 0.5)
	{
		out_color = vec4(0, 0, 0, 1.0);
	}
	else{
		vec3 view_dir = normalize(frag_pos - view_pos);
		vec3 f0 = vec3(0.04); 

		vec3 light_output = vec3(0.0);

		for(int i = 0;i < MAX_DIR_LIGHT; i++)
		{
			light_output += per_light_calc(vec3(point_lights[i].position) - frag_pos, point_lights[i].diffuse.rgb, mat, flat_normal, view_pos, f0);
		}

		for(int i = 0; i <  5; i++)
		{
			light_output += per_light_calc(dir_lights[i].direction.xyz, dir_lights[i].diffuse.rgb, mat, flat_normal, view_dir, f0);
		}
		out_color = vec4(light_output, 1.0);
	}

}
