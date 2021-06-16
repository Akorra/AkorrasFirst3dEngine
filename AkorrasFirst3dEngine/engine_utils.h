#ifndef __ENGINE_UTILS_HPP__
#define __ENGINE_UTILS_HPP__

#include <fstream>
#include <strstream>
#include <algorithm>
#include <vector>

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

	vec3d normalise()
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
	vec3d p[3]  = { vec3d(), vec3d(), vec3d() };
	wchar_t sym = L'a';
	short	col = 0;

	triangle()
	{
		p[0] = vec3d();
		p[1] = vec3d();
		p[2] = vec3d();
		sym = L'a';
		col = 0;
	}

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
		mat4x4(0.0f);
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
	
	mat4x4 Inverse()
	{
		//Should only be used for rotation/translation matrices
		mat4x4 matrix(0.0f);

		matrix.m[0][0] = m[0][0];	matrix.m[0][1] = m[1][0];	matrix.m[0][2] = m[2][0];	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m[0][1];	matrix.m[1][1] = m[1][1];	matrix.m[1][2] = m[2][1];	matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m[0][2];	matrix.m[2][1] = m[1][2];	matrix.m[2][2] = m[2][2];	matrix.m[2][3] = 0.0f;

		matrix.m[3][0] = -(m[3][0] * matrix.m[0][0] + m[3][1] * matrix.m[1][0] + m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m[3][0] * matrix.m[0][1] + m[3][1] * matrix.m[1][1] + m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m[3][0] * matrix.m[0][2] + m[3][1] * matrix.m[1][2] + m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;

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

	static mat4x4 PointAt(vec3d& pos, vec3d& target, vec3d& up)
	{
		vec3d _forward = target - pos;
		_forward = _forward.normalise();

		vec3d _up = up - ( _forward * ( up.dot(_forward) ) );
		_up = _up.normalise();

		vec3d _right = _up.cross(_forward);

		//Construct Dimensioning and Translation Matrix
		mat4x4 matrix(0.0f);
		matrix.m[0][0] =   _right.x;	matrix.m[0][1] =   _right.y;	matrix.m[0][2] =   _right.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] =      _up.x;	matrix.m[1][1] =      _up.y;	matrix.m[1][2] =      _up.z;	matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = _forward.x;	matrix.m[2][1] = _forward.y;	matrix.m[2][2] = _forward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] =      pos.x;	matrix.m[3][1] =      pos.y;	matrix.m[3][2] =      pos.z;	matrix.m[3][3] = 1.0f;
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

				triangle new_t;
				new_t.p[0] = verts[f[0] - 1];
				new_t.p[1] = verts[f[1] - 1];
				new_t.p[2] = verts[f[2] - 1];
			
				tris.push_back(new_t);
			}
		}

		return true;
	}
};

static vec3d IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& line_start, vec3d& line_end)
{
	plane_n = plane_n.normalise();
	float plane_d = -plane_n.dot(plane_n);
	float ad = line_start.dot(plane_n);
	float bd = line_end.dot(plane_n);

	float t = (-plane_d - ad) / (bd - ad);

	vec3d line_start2end = line_end - line_start;
	vec3d line_2intersect = line_start2end * t;

	return (line_start + line_2intersect);
}

//returns # of triangles returned by function
static int ClipTriangleAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& in_tri, triangle& out_tri1, triangle& out_tri2)
{
	// ensure plane normal is normal
	plane_p = plane_p.normalise();

	// Return signed shortest distance from point to plane, plane normal must be normalised
	auto dist = [&](vec3d& p)
	{
		vec3d n = p.normalise();
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.dot(plane_p));
	};

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	vec3d* inside_points[3];  int nInsidePointCount = 0;
	vec3d* outside_points[3]; int nOutsidePointCount = 0;

	// Get signed distance of each point in triangle to plane
	float d0 = dist(in_tri.p[0]);
	float d1 = dist(in_tri.p[1]);
	float d2 = dist(in_tri.p[2]);

	if (d0 >= 0)
		inside_points[nInsidePointCount++] = &in_tri.p[0];
	else
		outside_points[nOutsidePointCount++] = &in_tri.p[0];
	if (d1 >= 0)
		inside_points[nInsidePointCount++] = &in_tri.p[1];
	else
		outside_points[nOutsidePointCount++] = &in_tri.p[1];
	if (d2 >= 0)
		inside_points[nInsidePointCount++] = &in_tri.p[2];
	else
		outside_points[nOutsidePointCount++] = &in_tri.p[2];

	// Now classify triangle points, and break the input triangle into 
	// smaller output triangles if required. There are four possible
	// outcomes...
	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole triangle
		// It ceases to exist

		return 0; // No returned triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the triangle to simply pass through
		out_tri1 = in_tri;

		return 1; // Just the one returned original triangle is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Triangle should be clipped. As two points lie outside
		// the plane, the triangle simply becomes a smaller triangle

		// Copy appearance info to new triangle
		out_tri1.col = in_tri.col;
		out_tri1.sym = in_tri.sym;

		// The inside point is valid, so keep that...
		out_tri1.p[0] = *inside_points[0];

		// but the two new points are at the locations where the 
		// original sides of the triangle (lines) intersect with the plane
		out_tri1.p[1] = IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		out_tri1.p[2] = IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

		return 1; // Return the newly formed single triangle
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// Triangle should be clipped. As two points lie inside the plane,
		// the clipped triangle becomes a "quad". Fortunately, we can
		// represent a quad with two new triangles

		// Copy appearance info to new triangles
		out_tri1.col = in_tri.col;
		out_tri1.sym = in_tri.sym;

		out_tri2.col = in_tri.col;
		out_tri2.sym = in_tri.sym;

		// The first triangle consists of the two inside points and a new
		// point determined by the location where one side of the triangle
		// intersects with the plane
		out_tri1.p[0] = *inside_points[0];
		out_tri1.p[1] = *inside_points[1];
		out_tri1.p[2] = IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

		// The second triangle is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// triangle and the plane, and the newly created point above
		out_tri2.p[0] = *inside_points[1];
		out_tri2.p[1] = out_tri1.p[2];
		out_tri2.p[2] = IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

		return 2; // Return two newly formed triangles which form a quad
	}
}


#endif