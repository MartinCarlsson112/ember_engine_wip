#pragma once
#include <stdint.h>
#include <math.h>
#undef far
#undef near
#include <array>
#include <vector>

namespace math
{
	inline constexpr static float pi = 3.14159f;

	inline constexpr static float half_pi = pi / 2.0f;
	inline constexpr static float d2r = pi / 180.0f;
	inline constexpr static float r2d = 180.0f / pi;
	inline constexpr static float epsilon = 0.00001f;
};


struct float3
{
	inline float& operator[](int i) { return (&x)[i]; }
	inline constexpr float3() : x(0), y(0), z(0) {}
	constexpr float3(float x, float y, float z) : x(x), y(y), z(z) {}

	constexpr void operator+=(float3 b)
	{
		this->x += b.x;
		this->y += b.y;
		this->z += b.z;
	}

#pragma warning(disable : 4201)
	struct
	{
		float x, y, z;
	};
#pragma warning(default : 4201)
	const static float3 zero;
};

inline const float3 float3::zero = { 0,0,0 };


constexpr inline float3 operator-(float3 lhs, float3 rhs)
{
	return float3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

constexpr inline float3 operator*(float3 a, float b)
{
	return float3(a.x * b, a.y * b, a.z * b);
}

constexpr inline float3 operator*(float a, float3 b)
{
	return float3(a * b.x, a * b.y, a * b.z);
}

constexpr inline float3 operator*(float3 a, float3 b)
{
	return float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

constexpr inline float3 operator+(float3 a, float3 b)
{
	return float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
constexpr inline float3 operator+(float3 a, float b)
{
	return float3(a.x + b, a.y + b, a.z + b);
}


constexpr inline float3 operator-(float3 a)
{
	return float3(-a.x, -a.y, -a.z);
}


struct bezier
{
	float3 p1, p2, c1, c2;
};

struct hermite
{
	float3 p1, p2, c1, c2;
};


struct float4
{

	float4() :x(0), y(0), z(0), w(0) {}
	float4(float3 a, float w) : x(a.x), y(a.y), z(a.z), w(w) {}
	float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	float4(const float4& a) {
		x = a.x;
		y = a.y;
		z = a.z;
		w = a.w;
	}

	float x, y, z, w;

	const static float4 zero;
};

inline const float4 float4::zero = float4(0, 0, 0, 0);


inline float4 operator-(float4 lhs, float4 rhs)
{
	return float4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

inline float4 operator*(float4 a, float b)
{
	return float4(a.x * b, a.y * b, a.z * b, a.w * b);
}

inline float4 operator*(float a, float4 b)
{
	return float4(a * b.x, a * b.y, a * b.z, a * b.w);
}

inline float4 operator*(float4 a, float4 b)
{
	return float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline float4 operator+(float4 a, float4 b)
{
	return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline float4 operator+(float4 a, float b)
{
	return float4(a.x + b, a.y + b, a.z + b, a.w + b);
}


inline float4 operator-(float4 a)
{
	return float4(-a.x, -a.y, -a.z, -a.w);
}


struct float2
{
	inline float& operator[](int i) { return (&x)[i]; }
	inline constexpr float2() : x(0), y(0) {}
	constexpr float2(float x, float y) : x(x), y(y) {}

#pragma warning(disable : 4201)
	struct
	{
		float x, y;
	};
#pragma warning(default : 4201)
	const static float2 zero;
};

inline const float2 float2::zero = { 0,0 };


inline float2 operator-(float2 lhs, float2 rhs)
{
	return float2(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline float2 operator*(float2 a, float b)
{
	return float2(a.x * b, a.y * b);
}

inline float2 operator*(float a, float2 b)
{
	return float2(a * b.x, a * b.y);
}

inline float2 operator*(float2 a, float2 b)
{
	return float2(a.x * b.x, a.y * b.y);
}

inline float2 operator+(float2 a, float2 b)
{
	return float2(a.x + b.x, a.y + b.y);
}
inline float2 operator+(float2 a, float b)
{
	return float2(a.x + b, a.y + b);
}

inline float2 operator-(float2 a)
{
	return float2(-a.x, -a.y);
}


using float4x4 = float[16];

struct int2
{
	inline constexpr int2() : x(0), y(0) {}
	inline constexpr int2(int a, int b) :x(a), y(b) {}

	int x, y;
};


inline int2 operator/(int2 a, int b)
{
	return int2(a.x / b, a.y / b);
}

struct uint32_2
{
	inline constexpr uint32_2() : x(0), y(0) {}
	inline constexpr uint32_2(uint32_t a, uint32_t b) : x(a), y(b) {}

	uint32_t x, y;
};

struct int3
{

};


struct uint32_3
{
	inline uint32_t& operator[](int i) { return (&x)[i]; }
	inline constexpr uint32_3() : x(0), y(0), z(0) {}
	constexpr uint32_3(uint32_t x, uint32_t y, uint32_t z) : x(x), y(y), z(z) {}
#pragma warning(disable : 4201)
	struct
	{
		uint32_t x, y, z;
	};
#pragma warning(default : 4201)
	const static uint32_3 zero;
};

inline const uint32_3 uint32_3::zero = { 0,0,0 };


inline uint32_3 operator-(uint32_3 lhs, uint32_3 rhs)
{
	return uint32_3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline uint32_3 operator*(uint32_3 a, uint32_t b)
{
	return uint32_3(a.x * b, a.y * b, a.z * b);
}

inline uint32_3 operator*(uint32_t a, uint32_3 b)
{
	return uint32_3(a * b.x, a * b.y, a * b.z);
}

inline uint32_3 operator*(uint32_3 a, uint32_3 b)
{
	return uint32_3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline uint32_3 operator+(uint32_3 a, uint32_3 b)
{
	return uint32_3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline uint32_3 operator+(uint32_3 a, uint32_t b)
{
	return uint32_3(a.x + b, a.y + b, a.z + b);
}





struct uint32_4
{

	uint32_4() :x(0), y(0), z(0), w(0) {}
	uint32_4(uint32_3 a, uint32_t w) : x(a.x), y(a.y), z(a.z), w(w) {}
	uint32_4(uint32_t x, uint32_t y, uint32_t z, uint32_t w) : x(x), y(y), z(z), w(w) {}
	uint32_4(const uint32_4& a) {
		x = a.x;
		y = a.y;
		z = a.z;
		w = a.w;
	}

	uint32_t x, y, z, w;

	const static uint32_4 zero;
};

inline const uint32_4 uint32_4::zero = uint32_4(0, 0, 0, 0);


inline uint32_4 operator-(uint32_4 lhs, uint32_4 rhs)
{
	return uint32_4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

inline uint32_4 operator*(uint32_4 a, uint32_t b)
{
	return uint32_4(a.x * b, a.y * b, a.z * b, a.w * b);
}

inline uint32_4 operator*(uint32_t a, uint32_4 b)
{
	return uint32_4(a * b.x, a * b.y, a * b.z, a * b.w);
}

inline uint32_4 operator*(uint32_4 a, uint32_4 b)
{
	return uint32_4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline uint32_4 operator+(uint32_4 a, uint32_4 b)
{
	return uint32_4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
inline uint32_4 operator+(uint32_4 a, uint32_t b)
{
	return uint32_4(a.x + b, a.y + b, a.z + b, a.w + b);
}

struct quaternion
{
	union { 
		struct { float x;  float y; float z; float w; }; 
		struct { float3 vector; float scalar; };
		float v[4]; 
	};
	inline constexpr quaternion() : x(0), y(0), z(0), w(1) {}
	inline constexpr quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

inline constexpr quaternion operator+(const quaternion& a, const quaternion& b)
{
	return quaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline constexpr quaternion operator-(const quaternion& a, const quaternion& b)
{
	return quaternion(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline constexpr quaternion operator*(const quaternion& a, const float b)
{
	return quaternion(a.x * b, a.y * b, a.z * b, a.w * b);
}

inline constexpr quaternion operator-(const quaternion& a)
{
	return quaternion(-a.x, -a.y, -a.z, -a.w);
}


inline bool operator==(const quaternion& left, const quaternion& right)
{
	return (fabsf(left.x - right.x) <= math::epsilon && fabsf(left.y - right.y) <= math::epsilon && fabsf(left.z - right.z) <= math::epsilon && fabsf(left.w - right.w) <= math::epsilon);
}
inline bool operator!=(const quaternion& left, const quaternion& right)
{
	return !(left == right);
}

namespace math
{

	inline float dot(const float3& a, const float3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	inline float dot(const float4& a, const float4& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * a.z;
	}

	inline float dot(const quaternion& a, const quaternion& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * a.z;
	}

	inline float sqr_length(const float3& p)
	{
		return dot(p, p);
	}

	inline float sqr_length(const quaternion& q)
	{
		return dot(q, q);
	}

	inline float length(const quaternion& q)
	{
		float len = sqr_length(q);

		if (len < epsilon)
		{
			return 0.0f;
		}

		return sqrtf(len);
	}

	inline float length(const float3& p)
	{
		float len = sqr_length(p);
		if (len < epsilon)
		{
			return 0.0f;
		}
		return sqrtf(len);
	}

	inline float3 normalize(const float3& a)
	{
		return a * (1.0f / sqrtf(dot(a, a)));
	}

	inline float3 cross(const float3& a, const float3& b)
	{
		float3 res = float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
		return res;
	}

	inline float3 project(const float3& a, const float3& b)
	{
		float blen = length(b);
		if (blen < epsilon)
		{
			return float3();
		}
		float scale = dot(a, b) / blen;
		return b * scale;
	}

	inline float3 reflect(const float3& a, const float3& b)
	{
		float blen = length(b);
		if(blen < epsilon)
		{
			return float3();
		}
		float scale = dot(a, b) / blen;
		float3 proj2 = b * (scale * 2);
		return a - proj2;
	}

	inline float3 reject(const float3& a, const float3& b)
	{
		float3 projection = project(a, b);
		return a - projection;
	}	

	inline float angle_between(const float3& a, const float3& b)
	{
		float lensqr_a = sqr_length(a);
		float lensqr_b = sqr_length(b);
		if (lensqr_a < epsilon || lensqr_b < epsilon)
		{
			return 0.0f;
		}
		float val = dot(a, b);
		float len = sqrtf(lensqr_a) * sqrtf(lensqr_b);
		return acosf(val / len);
	}

	inline float abs(float x)
	{
		return (float)((unsigned int)x & 0x7FFFFFFF);
	}

	inline bool equals(const float3& a, const float3& b)
	{
		bool x = abs(a.x - b.x) < epsilon;
		bool y = abs(a.y - b.y) < epsilon;
		bool z = abs(a.z - b.z) < epsilon;
		return x && y && z;
	}

	inline const float clamp(const float v, const float lo, const float hi)
	{
		return (v < lo) ? lo : (hi < v) ? hi : v;
	}

	inline float deg2rad(float value) {
		return value * d2r;
	}
	inline float3 deg2rad(const float3& deg)
	{
		return float3(d2r * deg.x, d2r * deg.y, d2r * deg.z);
	}



	inline float3 lerp(const float3& a, const float3& b, const float alpha)
	{
		return a + (b - a) * alpha;
	}

	inline float3 interpolate(const bezier& curve, const float alpha)
	{
		return curve.p1 * ((1 - alpha) * (1 - alpha) * (1 - alpha)) 
			+ curve.c1 * (3.0f * ((1 - alpha) * (1 - alpha)) * alpha) 
			+ curve.c2 * (3.0f * (1 - alpha) * (alpha * alpha)) 
			+ curve.p2 * (alpha * alpha * alpha);
	}

	inline float3 interpolate(const hermite& curve, const float alpha)
	{
		return      
			curve.p1 * ((1.0f + 2.0f * alpha) * ((1.0f - alpha) * (1.0f - alpha)))
			+ curve.c1 * (alpha * ((1.0f - alpha) * (1.0f - alpha)))
			+ curve.p2 * ((alpha * alpha) * (3.0f - 2.0f * alpha))
			+ curve.c2 * ((alpha * alpha) * (alpha - 1.0f));
	}

	inline void perspective(float fov, float aspect, float near, float far, float4x4& output)
	{
		float f = 1.0f / tanf(deg2rad(fov * 0.5f));
		output[0] = f / aspect;
		output[1] = 0.0f;
		output[2] = 0.0f;
		output[3] = 0.0f;

		output[4] = 0.0f;
		output[5] = -f;
		output[6] = 0.0f;
		output[7] = 0.0f;
		
		output[8] = 0.0f;
		output[9] = 0.0f;
		output[10] = far / (near - far);
		output[11] = -1;
		
		output[12] = 0.0f;
		output[13] = 0.0f;
		output[14] = (near * far) / (near - far);
		output[15] = 0.0f;
	}

	inline float3 mul(float4x4 const& left, float3 right)
	{
		float x = dot(float3(left[0], left[1], left[2]), right);
		float y = dot(float3(left[4], left[5], left[6]), right);
		float z = dot(float3(left[8], left[9], left[10]), right);
		return float3(x, y, z);
	}

	inline quaternion angle_axis()
	{
		return quaternion();
	}

	inline quaternion from_to()
	{
		return quaternion();
	}

	inline bool same_orientation(const quaternion& l, const quaternion&r)
	{
		return false;
	}

	inline void mul(float4x4 const& left,
		float4x4 const& right, float4x4& result) {
		result[0]  = left[0] * right[0] + left[4] * right[1] + left[8] * right[2] + left[12] * right[3];
		result[1]  = left[1] * right[0] + left[5] * right[1] + left[9] * right[2] + left[13] * right[3];
		result[2]  = left[2] * right[0] + left[6] * right[1] + left[10] * right[2] + left[14] * right[3];
		result[3]  = left[3] * right[0] + left[7] * right[1] + left[11] * right[2] + left[15] * right[3];
		result[4]  = left[0] * right[4] + left[4] * right[5] + left[8] * right[6] + left[12] * right[7];
		result[5]  = left[1] * right[4] + left[5] * right[5] + left[9] * right[6] + left[13] * right[7];
		result[6]  = left[2] * right[4] + left[6] * right[5] + left[10] * right[6] + left[14] * right[7];
		result[7]  = left[3] * right[4] + left[7] * right[5] + left[11] * right[6] + left[15] * right[7];
		result[8]  = left[0] * right[8] + left[4] * right[9] + left[8] * right[10] + left[12] * right[11];
		result[9]  = left[1] * right[8] + left[5] * right[9] + left[9] * right[10] + left[13] * right[11];
		result[10] = left[2] * right[8] + left[6] * right[9] + left[10] * right[10] + left[14] * right[11];
		result[11] = left[3] * right[8] + left[7] * right[9] + left[11] * right[10] + left[15] * right[11];
		result[12] = left[0] * right[12] + left[4] * right[13] + left[8] * right[14] + left[12] * right[15];
		result[13] = left[1] * right[12] + left[5] * right[13] + left[9] * right[14] + left[13] * right[15];
		result[14] = left[2] * right[12] + left[6] * right[13] + left[10] * right[14] + left[14] * right[15];
		result[15] = left[3] * right[12] + left[7] * right[13] + left[11] * right[14] + left[15] * right[15];
	}
	
	inline bool inverse_matrix(float4x4 const & m, float4x4 & inverse_result)
	{
		float4x4 inv;
		float det;
		int i;

		inv[0] = m[5] * m[10] * m[15] -
			m[5] * m[11] * m[14] -
			m[9] * m[6] * m[15] +
			m[9] * m[7] * m[14] +
			m[13] * m[6] * m[11] -
			m[13] * m[7] * m[10];

		inv[4] = -m[4] * m[10] * m[15] +
			m[4] * m[11] * m[14] +
			m[8] * m[6] * m[15] -
			m[8] * m[7] * m[14] -
			m[12] * m[6] * m[11] +
			m[12] * m[7] * m[10];

		inv[8] = m[4] * m[9] * m[15] -
			m[4] * m[11] * m[13] -
			m[8] * m[5] * m[15] +
			m[8] * m[7] * m[13] +
			m[12] * m[5] * m[11] -
			m[12] * m[7] * m[9];

		inv[12] = -m[4] * m[9] * m[14] +
			m[4] * m[10] * m[13] +
			m[8] * m[5] * m[14] -
			m[8] * m[6] * m[13] -
			m[12] * m[5] * m[10] +
			m[12] * m[6] * m[9];

		inv[1] = -m[1] * m[10] * m[15] +
			m[1] * m[11] * m[14] +
			m[9] * m[2] * m[15] -
			m[9] * m[3] * m[14] -
			m[13] * m[2] * m[11] +
			m[13] * m[3] * m[10];

		inv[5] = m[0] * m[10] * m[15] -
			m[0] * m[11] * m[14] -
			m[8] * m[2] * m[15] +
			m[8] * m[3] * m[14] +
			m[12] * m[2] * m[11] -
			m[12] * m[3] * m[10];

		inv[9] = -m[0] * m[9] * m[15] +
			m[0] * m[11] * m[13] +
			m[8] * m[1] * m[15] -
			m[8] * m[3] * m[13] -
			m[12] * m[1] * m[11] +
			m[12] * m[3] * m[9];

		inv[13] = m[0] * m[9] * m[14] -
			m[0] * m[10] * m[13] -
			m[8] * m[1] * m[14] +
			m[8] * m[2] * m[13] +
			m[12] * m[1] * m[10] -
			m[12] * m[2] * m[9];

		inv[2] = m[1] * m[6] * m[15] -
			m[1] * m[7] * m[14] -
			m[5] * m[2] * m[15] +
			m[5] * m[3] * m[14] +
			m[13] * m[2] * m[7] -
			m[13] * m[3] * m[6];

		inv[6] = -m[0] * m[6] * m[15] +
			m[0] * m[7] * m[14] +
			m[4] * m[2] * m[15] -
			m[4] * m[3] * m[14] -
			m[12] * m[2] * m[7] +
			m[12] * m[3] * m[6];

		inv[10] = m[0] * m[5] * m[15] -
			m[0] * m[7] * m[13] -
			m[4] * m[1] * m[15] +
			m[4] * m[3] * m[13] +
			m[12] * m[1] * m[7] -
			m[12] * m[3] * m[5];

		inv[14] = -m[0] * m[5] * m[14] +
			m[0] * m[6] * m[13] +
			m[4] * m[1] * m[14] -
			m[4] * m[2] * m[13] -
			m[12] * m[1] * m[6] +
			m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] +
			m[1] * m[7] * m[10] +
			m[5] * m[2] * m[11] -
			m[5] * m[3] * m[10] -
			m[9] * m[2] * m[7] +
			m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] -
			m[0] * m[7] * m[10] -
			m[4] * m[2] * m[11] +
			m[4] * m[3] * m[10] +
			m[8] * m[2] * m[7] -
			m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] +
			m[0] * m[7] * m[9] +
			m[4] * m[1] * m[11] -
			m[4] * m[3] * m[9] -
			m[8] * m[1] * m[7] +
			m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] -
			m[0] * m[6] * m[9] -
			m[4] * m[1] * m[10] +
			m[4] * m[2] * m[9] +
			m[8] * m[1] * m[6] -
			m[8] * m[2] * m[5];

		det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		if (det == 0)
		{
			return false;
		}
			
		det = 1.0f / det;

		for (i = 0; i < 16; i++)
		{
			inverse_result[i] = inv[i] * det;
		}
		return true;
	}

	inline void rotation_matrix(const float angle, float3 const& axis, float4x4& output)
	{
		float x = axis.x, y = axis.y, z = axis.z;

		float angle_in_rad = angle;

		const float c = cosf(angle_in_rad);
		const float one_minus_c = 1.0f - c;
		const float s = sinf(angle_in_rad);

		output[0] = x * x * one_minus_c + c;
		output[1] = y * x * one_minus_c - z * s;
		output[2] = z * x * one_minus_c + y * s;
		output[3] = 0.0f;
		output[4] = x * y * one_minus_c + z * s;
		output[5] = y * y * one_minus_c + c;
		output[6] = z * y * one_minus_c - x * s;
		output[7] = 0.0f;
		output[8] = x * z * one_minus_c - y * s;
		output[9] = y * z * one_minus_c + x * s;
		output[10] = z * z * one_minus_c + c;
		output[11] = 0.0f;
		output[12] = 0.0f;
		output[13] = 0.0f;
		output[14] = 0.0f;
		output[15] = 1.0f;
	}

	inline void translate(const float3& translation, float4x4& output)
	{
		output[0] = 1.0f;
		output[1] = 0;
		output[2] = 0;
		output[3] = 0;
		output[4] = 0;
		output[5] = 1;
		output[6] = 0;
		output[7] = 0.0f;
		output[8] = 0;
		output[9] = 0;
		output[10] = 1;
		output[11] = 0.0f;
		output[12] = translation.x;
		output[13] = translation.y;
		output[14] = translation.z;
		output[15] = 1.0f;
	}

	inline void lookat_matrix(const float3& position, const float3& right, const float3& forward, const float3& up, float4x4& output)
	{
		output[0] = right.x;
		output[1] = up.x;
		output[2] = -forward.x;
		output[3] = 0.0f;

		output[4] = right.y;
		output[5] = up.y;
		output[6] = -forward.y;
		output[7] = 0.0f;

		output[8] = right.z;
		output[9] = up.z;
		output[10] = -forward.z;
		output[11] = 0.0f;

		output[12] = dot(position, right);
		output[13] = dot(position, up);
		output[14] = dot(position, forward);
		output[15] = 1.0f;
	}





	constexpr static int perlin_hash[] = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	struct improved_noise {
		static std::vector<int> p;

		static void init()
		{
			p.resize(512);
			for (size_t i = 0; i < 256; i++) p[256U + i] = p[i] = perlin_hash[i];
		}

		static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
		static float lerp(float t, float a, float b) { return a + t * (b - a); }
		static float grad(int hash, float x, float y, float z) {
			int h = hash & 15;                      
			float u = h < 8 ? x : y,                 
				v = h < 4 ? y : h == 12 || h == 14 ? x : z;
			return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
		}
		static float noise(float3 pos) {
			int x = (int)floorf(pos.x) & 255;                 
			int y = (int)floorf(pos.y) & 255;                  
			int z = (int)floorf(pos.z) & 255;
			pos.x -= floorf(float(x));                                
			pos.y -= floorf(float(y));                                
			pos.z -= floorf(float(z));
			float u = fade(pos.x);                               
			float v = fade(pos.y);                            
			float w = fade(pos.z);
			int A = p[x] +y;
			int AA = p[A] +z;
			int AB = p[A + 1] + z;
	
			int B = p[x + 1] + y;
			int BA = p[B] + z, BB = p[B + 1] + z; 

			return lerp(w, lerp(v, lerp(u, grad(p[AA], pos.x, pos.y, pos.z),
				grad(p[BA], pos.x - 1, pos.y, pos.z)), 
				lerp(u, grad(p[AB], pos.x, pos.y - 1, pos.z),  
					grad(p[BB], pos.x - 1, pos.y - 1, pos.z))),
				lerp(v, lerp(u, grad(p[AA + 1], pos.x, pos.y, pos.z - 1), 
					grad(p[BA + 1], pos.x - 1, pos.y, pos.z - 1)),
					lerp(u, grad(p[AB + 1], pos.x, pos.y - 1, pos.z - 1),
						grad(p[BB + 1], pos.x - 1, pos.y - 1, pos.z - 1))));
		}

	};

	inline std::vector<int> improved_noise::p;
}