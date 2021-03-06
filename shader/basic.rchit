#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hit_value;
layout(location = 2) rayPayloadEXT bool shadowed;
layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;

struct light_data
{
    vec4 pos;
    vec4 color;
    vec4 dir;
};

struct point_light_data
{
    vec4 pos;
    vec4 color;
};


layout(binding = 3, set = 0) uniform lbo
{
    light_data data[5];
    point_light_data point_lights[5];
} lights;

void main()
{
    float tmin = 0.0001;
	float tmax = 10000.0;
	vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	shadowed = true;  

    //TODO: Soft shadows
    //TODO: multiple rays per lights

    vec3 shadow_output = vec3(0, 0, 0);
    float light_count = 0.0;
    for(int i =0 ; i< 1; i++)
    {
        if(lights.data[i].color.w != 0.0)
        {
	        traceRayEXT(tlas, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 1, 0, 1, origin, tmin, -normalize(vec3(lights.data[i].dir)), tmax, 2);
        }
        if(!shadowed)
        {
            shadow_output += vec3(1.0, 1.0, 1.0);
            shadowed = true;
        }
        else{
               light_count+=1.0;
        }
    }
 
//    for(int i =0 ; i< 5; i++)
//    {
//        if(lights.point_lights[i].color.w != 0.0)
//        {
//
//
//	        traceRayEXT(tlas, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 1, 0, 1, origin, tmin, -normalize(origin - vec3(lights.point_lights[i].pos)), tmax, 2);
//        }
//        if(!shadowed)
//        {
//            shadow_output += vec3(1.0, 1.0, 1.0);
//            shadowed = true;
//        }
//        else{
//               light_count+=1.0;
//        }
//    }

    if( light_count> 0)
    {
        hit_value = shadow_output / light_count;
    }
    else
    {
        hit_value = vec3(1.0, 1.0, 1.0);
    }


}