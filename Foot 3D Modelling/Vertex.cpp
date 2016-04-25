#include "Vertex.h"



CVertex::CVertex()
{

}

CVertex::CVertex(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

const CVertex & CVertex::operator*(const float v) const
{
	CVertex r;
	r.x = x * v;
	r.y = x * v;
	r.z = x * v;
	return r;
}

const CVertex & CVertex::operator-(const CVertex &v) const
{
	CVertex r;
	r.x = x - v.x;
	r.y = x - v.y;
	r.z = x - v.z;
	return r;
}

const CVertex & CVertex::operator+(const CVertex &v) const
{
	CVertex r;
	r.x = x + v.x;
	r.y = x + v.y;
	r.z = x + v.z;
	return r;
}

bool const CVertex::operator==(const CVertex & v2) const
{
	return x == v2.x && y == v2.y && z == v2.z;
}
