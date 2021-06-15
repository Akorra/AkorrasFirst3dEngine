#pragma once

#include <fstream>
#include <strstream>
#include <algorithm>

struct vec3d
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	vec3d()
	{
		x = y = z = 0.0f;
		w = 1.0f;
	}

	vec3d(float a, float b, float c, float d=1.0f)
	{
		x = a; y = b; z = c; w = d;
	}

	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	vec3d normal()
	{
		return *this / length();
	}

	float dot(const vec3d& b)
	{
		return this->x * b.x + this->y * b.y + this->z * b.z;
	}

	float prod(const vec3d& b)
	{
		return (x * b.x + y * b.y + z * b.z);
	}

	vec3d cross(const vec3d& b)
	{
		return { this->y * b.z - this->z * b.y,
				 this->z * b.x - this->x * b.z,
				 this->x * b.y - this->y * b.x };
	}

	vec3d& operator+=(const vec3d& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		return *this;
	}

	vec3d& operator-=(const vec3d& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		return *this;
	}

	vec3d& operator*=(const float rhs)
	{
		this->x *= rhs;
		this->y *= rhs;
		this->z *= rhs;
		return *this;
	}

	vec3d& operator/=(const float rhs)
	{
		this->x /= rhs;
		this->y /= rhs;
		this->z /= rhs;
		return *this;
	}

	vec3d operator+(const vec3d& rhs)
	{
		vec3d r;
		r.x = this->x + rhs.x;
		r.y = this->y + rhs.y;
		r.z = this->z + rhs.z;
		return r;
	}

	vec3d operator-(const vec3d& rhs)
	{
		vec3d r;
		r.x = this->x - rhs.x;
		r.y = this->y - rhs.y;
		r.z = this->z - rhs.z;
		return r;
	}

	vec3d operator*(float rhs)
	{
		vec3d r;
		r.x = this->x * rhs;
		r.y = this->y * rhs;
		r.z = this->z * rhs;
		return r;
	}

	vec3d operator/(float rhs)
	{
		vec3d r;
		r.x = this->x / rhs;
		r.y = this->y / rhs;
		r.z = this->z / rhs;
		return r;
	}
};

struct triangle
{
	vec3d p[3];
	wchar_t sym;
	short	col;

	triangle operator+(const vec3d& rhs)
	{
		triangle o;
		o.p[0] = this->p[0] + rhs;
		o.p[1] = this->p[1] + rhs;
		o.p[2] = this->p[2] + rhs;
		return o;
	}
};

struct mat4x4
{
	float m[4][4] = { 0.0f };

	mat4x4()
	{
		m[4][4] = { 0.0f };
	}

	mat4x4(float d)
	{
		m[0][0] = d;
		m[1][1] = d;
		m[2][2] = d;
		m[3][3] = d;
	}

	vec3d operator*(vec3d v) {
		vec3d o;
		o.x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + v.w * m[3][0];
		o.y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + v.w * m[3][1];
		o.z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + v.w * m[3][2];
		o.w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3];

		return o;
	}

	triangle operator*(triangle tri)
	{
		triangle o;
		o.p[0] = *this * tri.p[0];
		o.p[1] = *this * tri.p[1];
		o.p[2] = *this * tri.p[2];
		return o;
	}

	mat4x4 operator*(mat4x4 m)
	{
		mat4x4 matrix(1.0f);
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = this->m[r][0] * m.m[0][c] + this->m[r][1] * m.m[1][c] + this->m[r][2] * m.m[2][c] + this->m[r][3] * m.m[3][c];
		return matrix;
	}

	static mat4x4 RotationX(const float rads)
	{
		mat4x4 matrix(1.0f);
		matrix.m[0][0] =  1.0f;
		matrix.m[1][1] =  cosf(rads);
		matrix.m[1][2] =  sinf(rads);
		matrix.m[2][1] = -sinf(rads);
		matrix.m[2][2] =  cosf(rads);
		matrix.m[3][3] =  1.0f;
		return matrix;
	}

	static mat4x4 RotationY(const float rads)
	{
		mat4x4 matrix(1.0f);
		matrix.m[0][0] = cosf(rads);
		matrix.m[0][2] = sinf(rads);
		matrix.m[2][0] = -sinf(rads);
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = cosf(rads);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	static mat4x4 RotationZ(const float rads)
	{
		mat4x4 matrix(1.0f);
		matrix.m[0][0] = cosf(rads);
		matrix.m[0][1] = sinf(rads);
		matrix.m[1][0] = -sinf(rads);
		matrix.m[1][1] = cosf(rads);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	static mat4x4 Translation(float x, float y, float z)
	{
		mat4x4 matrix(1.0f);
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	static mat4x4 Projection(float FovDegrees, float AspectRatio, float Near, float Far)
	{
		float FovRad = 1.0f / tanf(FovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix(1.0f);
		matrix.m[0][0] = AspectRatio * FovRad;
		matrix.m[1][1] = FovRad;
		matrix.m[2][2] = Far / (Far - Near);
		matrix.m[3][2] = (-Far * Near) / (Far - Near);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}
};

struct mesh
{
	std::vector<triangle> tris;

	bool LoadFromObjectFile(std::string sFilename)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		// Local cache of vertices
		std::vector<vec3d> verts;
		while (!f.eof())
		{
			char line[128]; // assuming file has no line with more than 128 characters
			f.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;
			if (line[0] == 'v')
			{
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}
			else if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}

		return true;
	}
};