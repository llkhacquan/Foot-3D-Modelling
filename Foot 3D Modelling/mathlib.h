#ifndef _MATHLIB_H_
#define _MATHLIB_H_

#define PI 3.141592654					// close enuf		
#define DTOR(deg) ((deg)*0.017453292)	// degrees to radians   
#define RTOD(rad) ((rad)*57.29577951)   // radians to degrees   
#define RADF 57.29577951				//180.0 / PI 

//function prototypes
float vecLen(float *v);
float cosAng(float *v1,float *v2);
float noDirectionMult(float *v1,float *v2);
double vecLen(double *v);
double noDirectionMult(double *v1,double *v2);

#endif