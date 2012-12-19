#pragma once

#pragma pack(push, 1)

// unions don't like structs with assignment operators
struct _vect3d
{
	float x;
	float y;
	float z;
};

struct vect3d
{
	_vect3d vect;
	vect3d() {vect.x = 0; vect.y = 0; vect.z = 0; }
	vect3d(const vect3d& other) : vect(other.vect){}
	vect3d(const _vect3d& other) : vect(other) {}
	
	vect3d& operator=(const _vect3d& rhs)
	{
		vect.x = rhs.x;
		vect.y = rhs.y;
		vect.z = rhs.z;
		return *this;
	}

	vect3d& operator=(const vect3d& rhs)
	{
		vect.x = rhs.vect.x;
		vect.y = rhs.vect.y;
		vect.z = rhs.vect.z;
		return *this;
	}
};

inline bool operator==(const _vect3d& lhs, const _vect3d& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline bool operator==(const vect3d& lhs, const vect3d& rhs)
{
	return lhs.vect == rhs.vect;
}

inline bool operator==(const _vect3d& lhs, const vect3d& rhs)
{
	return lhs == rhs.vect;
}

inline bool operator==(const vect3d& lhs, const _vect3d& rhs)
{
	return rhs == lhs.vect;
}

#pragma pack(pop)