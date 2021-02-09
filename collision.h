#pragma once
#include "mmath.h"
#include "components.h"
#include "array_util.h"

enum class collider_type
{
	box,
	sphere,
	capsule,
	mesh
};

struct collider
{
	float4x4 local_to_world;
	float4x4 local_to_world_inverse;
	collider_type type;
	union
	{
		float x_size;
		float radius;
	};

	union
	{
		float y_size;
		float y_base;
	};

	union
	{
		float z_size;
		float y_cap;
	};

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
	node* nodes;
	int node_count;
	int root_index;
};

//Todo: build dynamic aabb

struct support_data
{
	float3 pos;
	collider col;
};

struct contact_data
{
	float3 world_space_point_a;
	float3 world_space_point_b;
	float3 normal;
	float3 tangent_a;
	float3 tangent_b;
	float depth;
};

struct support_point
{
	float3 v;
	float3 a_support;
	float3 b_support;
	int unique_id;
};


constexpr float3 sphere_support(float3& search, const support_data& data)
{
	float3 pos = data.pos;
	float radius = data.col.radius;
	float3 d = math::normalize(search);
	return pos + radius * math::normalize(d);
}

constexpr float3 capsule_support(float3& search, const support_data& data)
{
	float4x4 mat = data.col.local_to_world;
	search = math::mul(data.col.local_to_world_inverse, search);

	float3 searchxz = float3(search.x, 0, search.z);
	float3 search_result = math::normalize(searchxz) * data.col.radius;
	search_result.y = (search.y > 0) ? data.col.y_cap : data.col.y_base;

	return math::mul(mat, search_result) + data.pos;
}

constexpr float3 box_support(const float3& search, const support_data& data)
{
	float4x4 mat = data.col.local_to_world;
	float3 result = float3(0, 0, 0);

	float3 size = float3(data.col.x_size, data.col.y_size, data.col.z_size);

	float3 vertices[] =
	{
		float3(-size.x, -size.y, -size.z),
		float3(size.x, -size.y, -size.z),
		float3(size.x, size.y, -size.z),
		float3(-size.x, size.y, -size.z),
		float3(-size.x, -size.y, size.z),
		float3(size.x, -size.y, size.z),
		float3(size.x, size.y, size.z),
		float3(-size.x, size.y, size.z),
	};


	float3 d;
	float maxDot = -std::numeric_limits<double>::infinity();
	result = float3(0,0,0);

	for (int i = 0; i < COUNT_OF(vertices); i++)
	{
		d = math::mul(mat, vertices[i]); 
		float dot = math::dot(d, search);
		if (dot > maxDot)
		{
			maxDot = dot;
			result = d;
		}
	}
	return result;
}


constexpr support_point support(float3& search, const support_data& a_data, const support_data& b_data, int& unique_id)
{
	float3 negative_search = -search;
	float3 temp_a = float3(0, 0, 0);
	float3 temp_b = float3(0, 0, 0);
	support_point assignment;

	switch (a_data.col.type)
	{
	case collider_type::box:
	{
		temp_a = box_support(negative_search, a_data);
	} break;
	case collider_type::sphere:
	{		
		temp_a = sphere_support(negative_search, a_data);
	} break;
	case collider_type::capsule:
	{
		temp_a = capsule_support(negative_search, a_data);
	} break;
	case collider_type::mesh:
	{
		//TODO: implement mesh support
	} break;
	default:
	{

	} break;
	}

	switch (b_data.col.type)
	{
	case collider_type::box:
	{
		temp_b = box_support(search, b_data);
	} break;
	case collider_type::sphere:
	{
		temp_b = sphere_support(search, b_data);
	} break;
	case collider_type::capsule:
	{
		temp_b = capsule_support(search, b_data);
	} break;
	case collider_type::mesh:
	{
		//TODO: implement mesh support
	} break;
	default:
	{

	} break;
	}
	assignment.unique_id = unique_id++;
	assignment.a_support = temp_a;
	assignment.b_support = temp_b;
	assignment.v = temp_b - temp_a;
	return assignment;
}


struct simplex
{
	int n;
	support_point a, b, c, d;
};


struct triangle
{
	support_point a, b, c;
	float3 n;

	triangle(const support_point& a, const support_point& b, const support_point& c)
	{
		this->a = a;
		this->b = b;
		this->c = c;
		n = math::normalize(math::cross((b.v - a.v), (c.v - a.v)));
	}
};

struct edge
{
	support_point a_point;
	support_point b_point;
};

namespace epa
{
	const int MAX_NUM_ITER = 64;
	const float GROWTH_TOLERANCE = 0.0001f;

	void add_edge(std::vector<edge>& edges, const support_point& a, const support_point& b)
	{
		for (int i = 0; i < edges.size(); i++)
		{
			if (edges[i].a_point.unique_id == b.unique_id && edges[i].b_point.unique_id == a.unique_id)
			{
				if (edges.size() > 1)
				{
					std::iter_swap(edges.begin() + i, edges.end() - 1);
					edges.pop_back();
				}
				else
				{
					edges.clear();
				}
			}
		}
		edges.push_back(edge{ a, b });
	}

	constexpr bool is_valid(float value)
	{
		return !isinf(value) && !isnan(value);
	}
	constexpr float3 barycentric(const float3& p, const float3& a, const float3& b, const float3& c)
	{
		float3 v0 = b - a, v1 = c - a, v2 = p - a;
		float d00 = math::dot(v0, v0);
		float d01 = math::dot(v0, v1);
		float d11 = math::dot(v1, v1);
		float d20 = math::dot(v2, v0);
		float d21 = math::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;
		return float3(u, v, w);
	}

	bool calculate(const simplex& simp, const support_data& a_support, const support_data& b_support, int unique_id, contact_data& data)
	{
		std::vector<triangle> triangles
		{
			triangle(simp.a, simp.b, simp.c),
			triangle(simp.a, simp.c, simp.d),
			triangle(simp.a, simp.d, simp.b),
			triangle(simp.b, simp.d, simp.c),
		};

		int closest_triangle_index = 0;
		bool converged = false;
		for (int iter = 0; iter < MAX_NUM_ITER; iter++)
		{
			std::vector<edge> edges;

			float min_dist = FLT_MAX;
			for (int i = 0; i < triangles.size(); i++)
			{
				float dist = math::dot(triangles[i].n, triangles[i].a.v);
				if (dist < min_dist)
				{
					closest_triangle_index = i;
					min_dist = dist;
				}
			}
			float3 closest_triangle_normal = triangles[closest_triangle_index].n;
			support_point search = support(closest_triangle_normal, a_support, b_support, unique_id);


			float new_dist = math::dot(triangles[closest_triangle_index].n, search.v);
			float growth = new_dist - min_dist;
			if (growth < GROWTH_TOLERANCE)
			{
				converged = true;
				break;
			}
			for (int i = 0; i < triangles.size(); i++)
			{
				//can this face be seen from the new search direction? 
				if (math::dot(triangles[i].n, search.v - triangles[i].a.v) > 0)
				{
					support_point a = triangles[i].a;
					support_point b = triangles[i].b;
					support_point c = triangles[i].c;

					add_edge(edges, a, b);
					add_edge(edges, b, c);
					add_edge(edges, c, a);

					if (triangles.size() > 1)
					{
						std::iter_swap(triangles.begin() + i, triangles.end() - 1);
						triangles.pop_back();
					}
					else
					{
						triangles.clear();
					}
					i--;
				}
			}

			// create new triangles from the edges in the edge list
			for (int i = 0; i < edges.size(); i++)
			{
				support_point a = edges[i].a_point;
				support_point b = edges[i].b_point;
				triangles.push_back(triangle(search, a, b));
			}
		}

		triangle closest_triangle = triangles[closest_triangle_index];
		float dist_from_origin = math::dot(closest_triangle.n, closest_triangle.a.v);
		float3 uvw = barycentric(closest_triangle.n * dist_from_origin, closest_triangle.a.v, closest_triangle.b.v, closest_triangle.c.v);

		if (!is_valid(uvw.x) || !is_valid(uvw.y) || !is_valid(uvw.z))
		{
			return false;
		}

		if (math::abs(uvw.x) > 1 || math::abs(uvw.y) > 1 || math::abs(uvw.z) > 1)
		{
			return false;
		}

		float3 aPoint = float3(closest_triangle.a.a_support * uvw.x + closest_triangle.b.a_support * uvw.y + closest_triangle.c.a_support * uvw.z);
		float3 bPoint = float3(closest_triangle.a.b_support * uvw.x + closest_triangle.b.b_support * uvw.y + closest_triangle.c.b_support * uvw.z);

		//position of a + normal * depth;
		//position of b + -normal * depth;

		data = contact_data();
		data.normal = math::normalize(closest_triangle.n);
		data.depth = dist_from_origin;
		data.world_space_point_a = aPoint;
		data.world_space_point_b = bPoint;
		return converged;
	}
}


namespace gjk
{

	constexpr bool intersect(const support_data& a_data, const support_data& b_data, /*out*/ contact_data& data, int MAX_ITERATIONS = 64)
	{
		int unique_id = 0;

		float3 search = a_data.pos - b_data.pos;

		support_point d = support_point();
		support_point c = support(search, a_data, b_data, unique_id);

		search = -c.v;

		support_point b = support(search, a_data, b_data, unique_id);

		if (math::dot(b.v, search) < 0)
		{
			return false;
		}

		search = math::cross(math::cross(c.v - b.v, -b.v), c.v - b.v);
		if (search.x == 0 && search.y == 0 && search.z == 0)
		{
			search = math::cross(c.v - b.v, float3(1, 0, 0));
			if (math::equals(search, float3::zero))
			{
				search = math::cross(c.v - b.v, float3(0, 0, -1));
			}
		}

		int n = 2;

		for (int i = 0; i < MAX_ITERATIONS; i++)
		{
			support_point a = support(search, a_data, b_data, unique_id);
			if (math::dot(a.v, search) < 0)
			{
				return false;
			}
			n++;
			if (n == 3)
			{
				search = update3(a, b, c, d, n);
			}
			else if (update4(a, b, c, d, n, unique_id, search))
			{
				simplex simp = simplex{ n, a, b ,c ,d };
				return epa::calculate(simp, a_data, b_data, unique_id, data);
			}
		}
		return false;
	}

	constexpr float3 update3(support_point& a, support_point& b, support_point& c, support_point& d, int& n)
	{
		float3 normal = math::cross(b.v - a.v, c.v - a.v);
		float3 AO = -a.v;

		n = 2;
		if (math::dot(math::cross(b.v - a.v, normal), AO) > 0)
		{
			c = a;
			return math::cross(math::cross(b.v - a.v, AO), b.v - a.v);
		}

		if (math::dot(math::cross(normal, c.v - a.v), c.v - a.v) > 0)
		{
			b = a;
			return math::cross(math::cross(c.v - a.v, AO), c.v - a.v);
		}
		n = 3;
		if (math::dot(normal, AO) > 0)
		{
			d = c;
			c = b;
			b = a;
			return  normal;
		}
		d = b;
		b = a;
		return -normal;
	}

	constexpr bool update4(support_point& a, support_point& b, support_point& c, support_point& d, int& n, int uniqueId, float3& search)
	{
		float3 abc = math::cross(b.v - a.v, c.v - a.v);
		float3 acd = math::cross(c.v - a.v, d.v - a.v);
		float3 adb = math::cross(d.v - a.v, b.v - a.v);

		float3 ao = -a.v;
		n = 3;

		if (math::dot(abc, ao) > 0)
		{
			d = c;
			c = b;
			b = a;
			search = abc;
			return false;
		}

		if (math::dot(acd, ao) > 0)
		{
			b = a;
			search = acd;
			return false;
		}

		if (math::dot(adb, ao) > 0)
		{
			c = d;
			d = b;
			b = a;
			search = adb;
			return false;
		}
		return true;
	}
}