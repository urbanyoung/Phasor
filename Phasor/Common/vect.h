#pragma once

#pragma pack(push, 1)

struct vect2d
{
	float x;
	float y;

	vect2d() { x = 0; y = 0; }
	vect2d(const vect2d& other) : x(other.x), y(other.y){}

	vect2d& operator=(const vect2d& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	vect2d& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		return *this;
	}
};

// only used to check if a vector hasn't changed (fpu comparisons are bad)
inline bool operator==(const vect2d& lhs, const vect2d& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const vect2d& lhs, const vect2d& rhs)
{
	return !(lhs == rhs);
}

// unions don't like structs with assignment operators
/*struct _vect3d
{
	float x;
	float y;
	float z;
};*/

struct vect3d
{
	float x;
	float y;
	float z;

	vect3d() {x = 0; y = 0; z = 0; }
	vect3d(const vect3d& other) : x(other.x), y(other.y), z(other.z){}

	vect3d& operator=(const vect3d& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}

	vect3d& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}
};

// only used to check if a vector hasn't changed (fpu comparisons are bad)
inline bool operator==(const vect3d& lhs, const vect3d& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator!=(const vect3d& lhs, const vect3d& rhs)
{
	return !(lhs == rhs);
}

struct plane3d
{
	float x;
	float y;
	float z;
	float d;

	plane3d() { x = 0; y = 0; z = 0; d = 0; }
	plane3d(const plane3d& other) : x(other.x), y(other.y), z(other.z), d(other.d){}

	plane3d& operator=(const plane3d& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		d = rhs.d;
		return *this;
	}

	plane3d& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;
		d *= rhs;
		return *this;
	}
};

// only used to check if a plane hasn't changed (fpu comparisons are bad)
inline bool operator==(const plane3d& lhs, const plane3d& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.d == rhs.d;
}

inline bool operator!=(const plane3d& lhs, const plane3d& rhs)
{
	return !(lhs == rhs);
}

#pragma pack(pop)