#pragma once
class CVertex
{
public:
	float x, y, z;

	bool const operator==(const CVertex & v2) const;

	const CVertex & operator+(const CVertex &v) const;

	const CVertex & operator-(const CVertex &v) const;

	const CVertex &  operator*(const float v) const;

	CVertex(float _x, float _y, float _z);

	CVertex();
};

class CMesh {
public:
	int a, b, c;
};