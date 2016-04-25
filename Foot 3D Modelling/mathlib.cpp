#include <math.h>
#include "mathlib.h"

/*
function: VecLen
Caculates the length of the vector in 3D space
*/
float vecLen(float *v)
{
	return ((float)(sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])));
}

double vecLen(double *v)
{
	return ((double)(sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])));
}

/*
function: CosAng
Returns cosine of the angle between two vector
*/
float noDirectionMult(float *v1,float *v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

double noDirectionMult(double *v1,double *v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

/*
function: CosAng
Returns cosine of the angle between two vector
*/
float cosAng(float *v1,float *v2)
{
	float ang,a,b;

	a=vecLen(v1);
	b=vecLen(v2);
	
	ang=( v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2])/(a*b);
	return ang;
}