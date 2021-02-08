#pragma once
#include "mmath.h"


struct aabb
{
	float3 lower_bound;
	float3 upper_bound;
};

constexpr float area(aabb a)
{
	float3 d = a.upper_bound - a.lower_bound;
	return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
}

constexpr aabb aabb_union(aabb a, aabb b)
{
	aabb res;
	res.lower_bound = math::float3_min(a.lower_bound, b.lower_bound);
	res.upper_bound = math::float3_max(a.upper_bound, b.upper_bound);
	return res;
}

struct node
{
	aabb box;
	int object_index;
	int parent_index;
	int child1;
	int child2;
	bool is_leaf;
};

struct tree
{
	Node* nodes;
	int node_count;
	int root_index;
};


/*

(1) assign a Morton code for each primitive according to its centroid, 
(2) sort the Morton codes, 
(3) construct a binary radix tree, and 
(4) assign a bounding box for eachinternal node.

*/


//linear bvh construction


//gjk


//epa


//collision mesh constructor