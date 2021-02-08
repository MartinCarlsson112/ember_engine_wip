#pragma once
#include <vector>
#include "mmath.h"
#include <string>
#include <cassert>

struct quaternion_frame
{
	quaternion value = quaternion();
	quaternion in = quaternion();
	quaternion out = quaternion();
	float t = 0;
};

struct float3_frame
{
	float3 value = float3();
	float3 in = float3();
	float3 out = float3();
	float t = 0;
};

enum class interpolation_type
{
	constant,
	linear,
	cubic
};

struct float3_track
{
	float3_track() : type(interpolation_type::constant) {}
	std::vector<float3_frame> frames;
	interpolation_type type;

	float3_frame& operator[](size_t index)
	{
		assert(index < frames.size());
		return frames[index];
	}

	void resize(size_t size)
	{
		frames.resize(size);
	}
	inline size_t size() const
	{
		return frames.size();
	}

	float start_time() const
	{
		frames[0].t;
	}

	float end_time() const
	{
		assert(frames.size() > 0);
		frames[frames.size() - 1].t;
	}

	float3 sample(float t, bool loop)
	{
		switch (type)
		{
		case interpolation_type::constant: return sample_constant(t, loop);
		case interpolation_type::linear: return sample_linear(t, loop);
		case interpolation_type::cubic: return sample_cubic(t, loop);
		default:
			break;
		}
		assert(false);
		return sample_constant(t, loop);
	}

	int frame_index(float t, bool loop)
	{
		const int size = (int)frames.size();

		if (size <= 1)
		{
			return -1;
		}

		if (loop) {
			float startTime = frames[0].t;
			float endTime = frames[size_t(size) - 1].t;
			t = fmodf(t - startTime, endTime - startTime);
			if (t < 0.0f)
			{
				t += endTime - startTime;
			}
			t = t + startTime;
			return t;
		}
		else
		{
			if (t <= frames[0].t) {
				return 0;
			}
			if (t >= frames[size_t(size) - 2].t) {
				return size - 2;
			}

			for (int i = (int)size - 1; i >= 0; --i)
			{
				if (t >= frames[i].t)
				{
					return i;
				}
			}
		}
		return -1;
	}
	float adjust_to_fit_track(float t, bool loop)
	{
		const size_t size = frames.size();
		if (size <= 1)
		{
			return 0.0f;
		}
		float start_time = frames[0].t;
		float end_time = frames[size - 1].t;
		float duration = end_time - start_time;
		if (duration <= 0.0f)
		{
			return 0.0f;
		}

		if (loop)
		{
			t = fmodf(t - start_time, end_time - start_time);
			if (t < 0.0f)
			{
				t += end_time - start_time;
			}
			t = t + start_time;
		}
		else
		{
			if (t <= frames[0].t)
			{
				t = start_time;
			}
			if (t >= frames[size - 1].t)
			{
				t = end_time;
			}
		}
		return t;
	}

protected:
	float3 sample_constant(float t, bool loop)
	{
		int f = frame_index(t, loop);

		if (f == -1 || f >= frames.size())
		{
			return float3();
		}
		return frames[f].value;
	}
	float3 sample_linear(float t, bool loop)
	{
		int this_frame = frame_index(t, loop);
		if (this_frame == -1 || this_frame >= frames.size() - 1)
		{
			return float3();
		}

		int next_frame = this_frame + 1;

		float track_time = adjust_to_fit_track(t, loop);
		float this_time = frames[this_frame].t;
		float frame_delta = frames[next_frame].t - this_time;

		if (frame_delta <= 0.0f)
		{
			return float3();
		}

		float time = (track_time - this_time) / frame_delta;

		float3 start = frames[this_frame].value;
		float3 end = frames[next_frame].value;
		return math::lerp(start, end, time);
	}

	float3 sample_cubic(float t, bool loop)
	{
		int this_frame = frame_index(t, loop);
		if (this_frame == -1 || this_frame >= frames.size() - 1)
		{
			return float3();
		}
		int next_frame = this_frame + 1;
		float track_time = adjust_to_fit_track(t, loop);
		float this_time = frames[this_frame].t;
		float frame_delta = frames[next_frame].t - this_time;


		if (frame_delta <= 0.0f)
		{
			return float3();
		}

		float time = (track_time - this_time) / frame_delta;

		float3 point1 = frames[this_frame].value;
		float3 point2 = frames[next_frame].value;
		float3 slope1 = frames[this_frame].out * frame_delta;
		float3 slope2 = frames[next_frame].in * frame_delta;
		return hermite(time, point1, slope1, point2, slope2);
	}
	float3 hermite(float t, const float3& p1, float3& s1, const float3& p2, const float3& s2)
	{
		float tt = t * t;
		float ttt = tt * t;
		float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
		float h2 = -2.0f * ttt + 3.0f * tt;
		float h3 = ttt - 2.0f * tt + t;
		float h4 = ttt - tt;
		float3 result = p1 * h1 + p2 * h2 + s1 * h3 + s2 * h4;
		return result;
	}
};

struct quaternion_track
{
	quaternion_track() : type(interpolation_type::constant) {}
	std::vector<quaternion_frame> frames;
	interpolation_type type;

	quaternion_frame& operator[](size_t index)
	{
		assert(index < frames.size());
		return frames[index];
	}

	void resize(size_t size)
	{
		frames.resize(size);
	}
	inline size_t size() const
	{
		return frames.size();
	}

	float start_time() const
	{
		frames[0].t;
	}

	float end_time() const
	{
		assert(frames.size() > 0);
		frames[frames.size() - 1].t;
	}

	quaternion sample(float t, bool loop)
	{
		switch (type)
		{
		case interpolation_type::constant: return sample_constant(t, loop);
		case interpolation_type::linear: return sample_linear(t, loop);
		case interpolation_type::cubic: return sample_cubic(t, loop);
		default:
			break;
		}
		assert(false);
		return sample_constant(t, loop);
	}

	int frame_index(float t, bool loop)
	{
		const int size = (int)frames.size();

		if (size <= 1)
		{
			return -1;
		}

		if (loop) {
			float startTime = frames[0].t;
			float endTime = frames[size_t(size)- 1].t;
			t = fmodf(t - startTime, endTime - startTime);
			if (t < 0.0f)
			{
				t += endTime - startTime;
			}
			t = t + startTime;
			return t;
		}
		else
		{
			if (t <= frames[0].t) {
				return 0;
			}
			if (t >= frames[size_t(size) - 2].t) {
				return size - 2;
			}

			for (int i = (int)size - 1; i >= 0; --i)
			{
				if (t >= frames[i].t)
				{
					return i;
				}
			}
		}
		return -1;
	}
	float adjust_to_fit_track(float t, bool loop)
	{
		const size_t size = frames.size();
		if (size <= 1)
		{
			return 0.0f;
		}
		float start_time = frames[0].t;
		float end_time = frames[size - 1].t;
		float duration = end_time - start_time;
		if (duration <= 0.0f)
		{
			return 0.0f;
		}

		if (loop)
		{
			t = fmodf(t - start_time, end_time - start_time);
			if (t < 0.0f)
			{
				t += end_time - start_time;
			}
			t = t + start_time;
		}
		else
		{
			if (t <= frames[0].t)
			{
				t = start_time;
			}
			if (t >= frames[size - 1].t)
			{
				t = end_time;
			}
		}
		return t;
	}

	quaternion cast(float* value)
	{
		quaternion r = quaternion(value[0], value[1], value[2], value[3]);
		return math::normalize(r);
	}

protected:
	quaternion sample_constant(float t, bool loop) 
	{
		int f = frame_index(t, loop);

		if (f == -1 || f >= frames.size())
		{
			return quaternion();
		}
		return frames[f].value;
	}
	quaternion sample_linear(float t, bool loop)
	{
		int this_frame = frame_index(t, loop);
		if (this_frame == -1 || this_frame >= frames.size() - 1)
		{
			return quaternion();
		}

		int next_frame = this_frame + 1;

		float track_time = adjust_to_fit_track(t, loop);
		float this_time = frames[this_frame].t;
		float frame_delta = frames[next_frame].t - this_time;

		if (frame_delta <= 0.0f)
		{
			return quaternion();
		}

		float time = (track_time - this_time) / frame_delta;
		quaternion start = frames[this_frame].value;
		quaternion end = frames[next_frame].value;

		if (math::dot(start, end) < 0)  // Neighborhood
		{
			return math::normalize(math::mix(start, -end, time));
		}
		else
		{
			return math::normalize(math::mix(start, end, time)); //NLerp, not slerp
		}

	}
	quaternion sample_cubic(float t, bool loop)
	{
		int this_frame = frame_index(t, loop);
		if (this_frame == -1 || this_frame >= frames.size() - 1)
		{
			return quaternion();
		}
		int next_frame = this_frame + 1;
		float track_time = adjust_to_fit_track(t, loop);
		float this_time = frames[this_frame].t;
		float frame_delta = frames[next_frame].t - this_time;


		if (frame_delta <= 0.0f)
		{
			return quaternion();
		}

		float time = (track_time - this_time) / frame_delta;

		size_t flt_size = sizeof(float);
		quaternion point1 = frames[this_frame].value;
		quaternion point2 = frames[next_frame].value;
		quaternion slope1 = frames[this_frame].out * frame_delta;
		quaternion slope2 = frames[next_frame].in * frame_delta;
		return hermite(time, point1, slope1, point2, slope2);
	}

	quaternion hermite(float t, const quaternion& p1, quaternion& s1, const quaternion& p2, const quaternion& s2)
	{
		float tt = t * t;
		float ttt = tt * t;
		quaternion _p2 = p2;
		if (math::dot(p1, _p2) < 0)
		{
			_p2 = -_p2;
		}
		float h1 = 2.0f * ttt - 3.0f * tt + 1.0f;
		float h2 = -2.0f * ttt + 3.0f * tt;
		float h3 = ttt - 2.0f * tt + t;
		float h4 = ttt - tt;
		quaternion result = p1 * h1 + _p2 * h2 + s1 * h3 + s2 * h4;
		return math::normalize(result);
	}
};

struct transform_track
{
	quaternion_track rotation;
	float3_track position;
	uint32_t bone_index;
	float3_track scale;

	transform_track() :bone_index(0) {}

	float get_start_time() const
	{
		float res = 0.0f;
		bool is_set = false;
		if (position.size() > 1)
		{
			res = position.start_time();
			is_set = true;
		}

		if (rotation.size() > 1)
		{
			float rotation_start = rotation.start_time();

			if (rotation_start < res || !is_set)
			{
				res = rotation_start;
				is_set = true;
			}
		}

		if (scale.size() > 1)
		{
			float scale_start = scale.start_time();
			if (scale_start < res || !is_set)
			{
				res = scale_start;
				is_set = true;
			}
		}

		return res;
	}
	float get_end_time() const
	{
		float res = 0.0f;
		bool is_set = false;
		if (position.size() > 1)
		{
			res = position.end_time();
			is_set = true;
		}

		if (rotation.size() > 1)
		{
			float rotation_start = rotation.end_time();

			if (rotation_start > res || !is_set)
			{
				res = rotation_start;
				is_set = true;
			}
		}

		if (scale.size() > 1)
		{
			float scale_start = scale.end_time();
			if (scale_start > res || !is_set)
			{
				res = scale_start;
				is_set = true;
			}
		}

		return res;
	}
	bool is_valid()const
	{
		return position.size() > 1 || rotation.size() > 1 || scale.size() > 1;
	}

	transform sample(const transform& tf, float t, bool loop)
	{
		transform res = tf;
		if (position.size() > 1)
		{
			res.position = position.sample(t, loop);
		}
		if (rotation.size() > 1)
		{
			res.rotation = rotation.sample(t, loop);
		}
		if (scale.size() > 1)
		{
			res.scale = scale.sample(t, loop);
		}
		return res;
	}
};

struct pose
{
	pose(){}
	pose(const pose& p) { *this = p; }
	pose& operator=(const pose& p) 
	{ 
		if (&p == this)
		{
			return *this;
		}

		if (parents.size() != p.parents.size())
		{
			parents.resize(p.parents.size());
		}
		if (joints.size() != p.joints.size())
		{
			joints.resize(p.joints.size());
		}

		if (parents.size() != 0)
		{
			memcpy(&parents[0], &p.parents[0], sizeof(int) * parents.size());
		}

		if (joints.size() != 0)
		{
			memcpy(&joints[0], &p.joints[0], sizeof(transform) * joints.size());
		}
		return *this;
	}
	pose(uint32_t n_joints)
	{
		resize(n_joints);
	}

	transform operator[](size_t index) const
	{
		return get_global_transform(index);
	}

	bool operator==(const pose& other) const
	{
		const size_t s = size();
		if (s != other.size())
		{
			return false;
		}

		for (size_t i = 0; i < s; ++i)
		{
			transform self_local = joints[i];
			transform other_local = other.joints[i];

			int self_parent = parents[i];
			int other_parent = other.parents[i];

			if (self_parent != other_parent)
			{
				return false;
			}

			if (!math::equals(self_local.position, other_local.position))
			{
				return false;
			}
			if (!math::equals(self_local.scale, other_local.scale))
			{
				return false;
			}
			if (self_local.rotation != other_local.rotation)
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(const pose& other) const
	{
		return !(*this == other);
	}


	void resize(uint32_t new_size) {
		joints.resize(new_size);
		parents.resize(new_size);
	}

	size_t size() const
	{
		return joints.size();
	}

	int get_parent(size_t index)
	{
		return parents[index];
	}

	void set_parent(size_t index, int parent)
	{
		parents[index] = parent;
	}

	transform get_local_transform(size_t index)
	{
		return joints[index];
	}

	void set_local_transform(size_t index, const transform& t)
	{
		joints[index] = t;
	}

	transform get_global_transform(size_t index) const
	{
		transform res = joints[index];

		for (int i = parents[index]; i >= 0; i = parents[i])
		{
			res = math::combine(joints[i], res);
		}

		return res;
	}
	
	void get_matrices(std::vector<float4x4>& res)
	{
		const size_t size = this->size();
		if (res.size() != size)
		{
			res.resize(size);
		}
		
		for (size_t i = 0; i < size; ++i)
		{
			transform t = get_global_transform(i);
			math::to_float4x4(t, res[i]);
		}
	}
	std::vector<transform> joints;
	std::vector<int> parents;
};

struct clip
{
	clip() :loop(true), start_time(0.0f), end_time(0.0f), name("no name") {}

	transform_track& operator[](uint32_t index)
	{
		for (size_t i = 0, s = tracks.size(); i < s; ++i)
		{
			if (tracks[i].bone_index == index)
			{
				return tracks[i];
			}
		}
		tracks.push_back(transform_track());
		tracks[tracks.size() - 1].bone_index = index; 
		return tracks[tracks.size() - 1];
	}

	float adjust_time_to_fit(float t)
	{
		if (loop) 
		{
			float duration = end_time - start_time;      
			if (duration <= 0) {
				0.0f;
			}
			t = fmodf(t - start_time, end_time - start_time);

			if (t < 0.0f) 
			{
				t += end_time - start_time;
			}   t = t + start_time;
		}
		else { 

			if (t < start_time) 
			{
				t = start_time;
			}
			if (t > end_time) 
			{
				t = end_time;
			}
		}
		return t;
	}

	size_t size()
	{
		return tracks.size();
	}

	uint32_t get_id(size_t index) const
	{
		return tracks[index].bone_index;
	}

	void set_id(size_t index, uint32_t id)
	{
		tracks[index].bone_index = id;
	}

	float sample(pose& out, float t)
	{
		if (get_duration() == 0.0f)
		{
			return 0.0f;
		}

		t = adjust_time_to_fit(t);

		const size_t s = tracks.size();
		for (size_t i = 0; i < s; ++i)
		{
			size_t j = tracks[i].bone_index;

			transform local = out.get_local_transform(j);
			transform animated = tracks[i].sample(local, t, loop);
			out.set_local_transform(j, animated);
		}
		return t;
	}

	void recalculate_duration()
	{
		start_time = 0.0f;    
		end_time = 0.0f;    
		bool start_set = false;   
		bool end_set = false;   
		const size_t s = tracks.size();   
		for (unsigned int i = 0; i < s; ++i)
		{
			if (tracks[i].is_valid())
			{
				float startTime = tracks[i].get_start_time();
				float endTime = tracks[i].get_end_time();
				if (startTime < start_time || !start_set)
				{
					start_time = startTime;
					start_set = true;
				}
				if (endTime > end_time || !end_set)
				{
					end_time = endTime;
					end_set = true;
				}
			}
		}
	}

	float get_duration()
	{
		return end_time - start_time;
	}

	std::vector<transform_track> tracks;
	std::string name;
	float start_time;
	float end_time;
	bool loop;
};

struct rig
{	
	rig(){}
	rig(const pose& rest, const pose& bind, std::vector<std::string>& names) : rest_pose(rest), bind_pose(bind), joint_names(names)
	{ 
		update_inverse_bind_pose(); 
	}

	void update_inverse_bind_pose()
	{
		size_t size = bind_pose.size();  
		inv_bind_pose.resize(size);  
		for (size_t i = 0; i < size; ++i)
		{
			transform world = bind_pose.get_global_transform(i);
			math::to_float4x4(world, inv_bind_pose[i]);
			math::inverse_matrix(inv_bind_pose[i], inv_bind_pose[i]);
		}
	}

	void set(const pose& rest, const pose& bind, const std::vector<std::string>& names)
	{
		rest_pose = rest;
		bind_pose = bind;
		joint_names = names;
		update_inverse_bind_pose();
	}

	void add_joint(const transform& joint_transform_bind, const transform& joint_transform_rest, int parent_index, const std::string& name, const float4x4& inv_bind)
	{
		rest_pose.joints.push_back(joint_transform_rest);
		rest_pose.parents.push_back(parent_index);

		bind_pose.joints.push_back(joint_transform_bind);
		bind_pose.parents.push_back(parent_index);

		joint_names.push_back(name);
		inv_bind_pose.push_back(inv_bind);
	}

	pose rest_pose;
	pose bind_pose;

	std::vector<float4x4> inv_bind_pose;
	std::vector<std::string> joint_names;
};

struct ik_bone
{
	transform t;
	float length;
};

struct ik_chain
{
	std::vector<ik_bone> bones;
	transform start;
	float total_length;
	size_t size;
};


namespace ik_solver
{
	//Fabrik IK solver - very rudimentary no support for constraints yet... 
	inline void solve_ik_chain(ik_chain& chain, transform target)
	{
		float3 start_pos = chain.start.position;
		float3 target_pos = target.position;

		chain.bones[chain.size - 1].t.position = target_pos;
		for (int i = (int)chain.size - 2; i >= 0; --i)
		{
			float3 current = chain.bones[i].t.position;
			float3 next = chain.bones[size_t(i)+1].t.position;
			float3 dir = current - next;

			dir = math::normalize(dir);

			float3 offset = dir * chain.bones[size_t(i) + 1].length;

			chain.bones[i].t.position = next + offset;
		}

		for (int i = 1; i < chain.size; ++i)
		{
			float3 current = chain.bones[i].t.position;
			float3 prev = chain.bones[size_t(i) + 1].t.position;
			float3 dir = current - prev;

			dir = math::normalize(dir);

			float3 offset = dir * chain.bones[i].length;

			chain.bones[i].t.position = prev + offset;
		}

		quaternion to_rot = quaternion();
		float3 up = float3(0, 1, 0);
		for (size_t i = 0; i < chain.size - 1; i++)
		{
			to_rot = math::look_rotation(chain.bones[i].t.position - chain.bones[i+1].t.position, up);
			chain.bones[i].t.rotation = to_rot;
		}

		to_rot = math::look_rotation(chain.bones[chain.size - 1].t.position - target_pos, up);
		chain.bones[chain.size - 1].t.rotation = to_rot;
	}
};