#include "stdio.h"


#include "torch\Allocator.h"
#include "torch\Mat.h"
#include "torch\mx_lu_factor.h"
#include "torch\Mat_operations.h"

#include <time.h>
#include "mathlib.h"



//#define 
//bool NORMAL_RBF=false;

using namespace Torch;


//#define Nx 5

double distance(double *xi,double *xj) 
{
	int i;
	double temp=0;
	
	for (i=0;i<3;i++) {
		temp+=(xi[i]-xj[i])*(xi[i]-xj[i]);
	}
	return sqrt(temp);
}

double distanceByCoor(double *xi, double *xj, int whichCoor) {
	double d = xi[whichCoor] - xj[whichCoor];
	return d < 0 ? -d : d;
}

double h_function(double *xi,int j,double **x,double *minx)
{
	double temp=distance(xi,x[j]);
	double result=sqrt(temp*temp+minx[j]*minx[j]);
	//double result=exp(-temp);
	return result;
}

double h_functionByCoor(double *xi,int j,double **x,double *minx, int whichCoor) {
	double temp=distanceByCoor(xi,x[j],whichCoor);
	double result=sqrt(temp*temp+minx[j]*minx[j]);
	return result;
}

double f_function(int n,double *xi,double *w,double **x,double *minx,
					int whichCoor, const bool& normalRBF)
{
	int j;

	double result;

	if (normalRBF) {
		result=0.0f;
	}
	else {
		result=xi[whichCoor];
	}

	for (j=0;j<n;j++) {
		result+=w[j]*h_function(xi,j,x,minx);			
	}
	return result;
}

double f_functionByCoor(int n,double *xi,double *w,double **x,double *minx,
					int whichCoor, const bool& normalRBF)
{
	int j;

	double result;

	if (normalRBF) {
		result=0.0f;
	}
	else {
		result=xi[whichCoor];
	}

	for (j=0;j<n;j++) {
		result+=w[j]*h_functionByCoor(xi,j,x,minx,whichCoor);			
	}
	return result;
}

void calMinx(double **x, int size, double *minx)
{
	int i,j;

	for (i=0;i<size;++i) {
		minx[i]=10000;
		for (j=0;j<size;++j) {
			if (i<j || i>j) {
				if (distance(x[i],x[j])<minx[i]) {
					minx[i]=distance(x[i],x[j]);
				}
			}
		}
	}
}

void calMinxByCoor(double **x, int size, double *minx, int whichCoor)
{
	int i,j;

	for (i=0;i<size;++i) {
		minx[i]=10000;
		for (j=0;j<size;++j) {
			if (i<j || i>j) {
				if (distanceByCoor(x[i],x[j], whichCoor)<minx[i]) {
					minx[i]=distanceByCoor(x[i],x[j], whichCoor);
				}
			}
		}
	}
}

double traceF(Mat *a)
{
	int i;
	double result=0.0f;

	for (i=0;i<a->n;i++) {
		result+=a->ptr[i][i];	
	}
	return result;
}

double ndMult(Vec *a,Vec *b)
{
	int i;
	double result=0;

	for (i=0;i<a->n;i++) {
		result+=(a->ptr[i])*(b->ptr[i]);
	}
	return result;
}

void subVec(Vec *a,Vec *b,Vec *out)
{
	int i;

	for (i=0;i<a->n;i++) {
		out->ptr[i]=a->ptr[i]-b->ptr[i];
	}
}

void vecAdd(Vec *a,Vec* b,Vec *result)
{
	int i;
	for (i=0;i<a->n;i++) {
		result->ptr[i]=a->ptr[i]+b->ptr[i];
	}
}

void vecSub(Vec *a,Vec* b,Vec *result)
{
	int i;
	for (i=0;i<a->n;i++) {
		result->ptr[i]=a->ptr[i]-b->ptr[i];
	}
}

/*
Morph a face to another face
- correspondingSource,correspondingTarget: set of source and target corresponding vertices
- Nx: number of vertices in correspondingSource
- source: source vertices
- target: target vertices
- NSource: number of vertices in source

*/
void morph(double **correspondingSource,double **correspondingTarget,double **source, double **target,
			int Nx,int NSource,int whichCoor, const bool& normalRBF)
{
	Allocator* allocator=new Allocator;

	Mat h(Nx,Nx);
	Mat ht(Nx,Nx);
	Mat a(Nx,Nx);
	Mat temp(Nx,Nx);
	Mat temp1(Nx,Nx);
	Mat at1(Nx,Nx);
	int i,j;
	double lamda=0.1f;
	double gcv,lastgcv;
	double ng,yg;
	double error,ww;
	Vec tempv(Nx);
	Vec tempv1(Nx);
	Vec e(Nx);
	Vec b(Nx);

	Vec w(Nx);
		
	double *minx;	
	double *ytemp;
	double *xtemp;

	ytemp=new double[Nx];
	for (i=0;i<Nx;++i) {
		ytemp[i]=correspondingTarget[i][whichCoor];
	}
	Vec yvec(ytemp,Nx);

	xtemp=new double[Nx];
	for (i=0;i<Nx;++i) {
		xtemp[i]=correspondingSource[i][whichCoor];
	}
	Vec x(xtemp,Nx);
		

	minx=new double[Nx];
	
	calMinxByCoor(correspondingSource,Nx,minx, whichCoor);

	for (i=0;i<Nx;++i) {
		for (j=0;j<Nx;++j) {
			h.ptr[i][j]=h_function(correspondingSource[i],j,correspondingSource,minx);
		}
	}

	gcv=10;
	lastgcv=20;
	while (gcv-lastgcv>0.001 || lastgcv-gcv>0.001) 
	{
		temp1.zero();
		for (i=0;i<Nx;++i) {
			temp1.ptr[i][i]=1;
		}
		mxRealMulMat(lamda,&temp1,&temp);

		mxTrMat(&h,&ht);

		mxMatMulMat(&ht,&h,&temp1);

		mxMatAddMat(&temp1,&temp,&a);

		//b=ht*y
		//a=ht*h+lamda*I
		
		if (normalRBF) {
			mxMatMulVec(&ht,&yvec,&b);
		}
		else {
			vecSub(&yvec,&x,&tempv);
			mxMatMulVec(&ht,&tempv,&b);
		}

		//aw=b
		//mxSolve(&a,&b,&w);

		////////////////////////////////////////////////////////////////////
		//calculate gcv and lamda
		
		//a^-1
		mxInverse(&a,&at1);

		mxMatMulVec(&at1, &b, &w);
		//a^-2
		mxMatMulMat(&at1,&at1,&temp);


		//lamda*a^-2
		mxRealMulMat(lamda,&temp,&temp1);

		//a^-1 - lamda* a^-2  
		mxMatSubMat(&at1,&temp1,&temp);
		ng=traceF(&temp);
		yg=((double)Nx)-lamda*traceF(&at1);
		mxMatMulVec(&h,&w,&tempv1);
		subVec(&yvec,&tempv1,&tempv);
		
		error=ndMult(&tempv,&tempv);
		lastgcv=gcv;
		gcv=Nx*error/((Nx-yg)*(Nx-yg));
		
		mxMatMulVec(&at1,&w,&tempv);
		ww=ndMult(&w,&tempv);

		lamda=ng*error/(Nx-yg)/ww;

		//printf("Error: %f  ----    Lamda: %f \n",gcv,lamda);
		

		mxMatMulVec(&h,&w,&tempv1);
		subVec(&yvec,&tempv1,&tempv);

		error=ndMult(&tempv,&tempv);

		//printf("Error: %f \n",error);		
	}
	
	for (i=0;i<NSource;++i) {
		target[i][whichCoor]=f_function(Nx,source[i],w.ptr,correspondingSource,minx,whichCoor, normalRBF);
		//printf("(%f,%f,%f) ---> %f\n",source[i][0],source[i][1],source[i][2],target[i][whichCoor]);
	}
	
	delete allocator;
}

void morphByCoor(double **correspondingSource,double **correspondingTarget,double **source, double **target,
			int Nx,int NSource,int whichCoor, const bool& normalRBF)
{
	Allocator* allocator=new Allocator;

	Mat h(Nx,Nx);
	Mat ht(Nx,Nx);
	Mat a(Nx,Nx);
	Mat temp(Nx,Nx);
	Mat temp1(Nx,Nx);
	Mat at1(Nx,Nx);
	int i,j;
	double lamda=0.1f;
	double gcv,lastgcv;
	double ng,yg;
	double error,ww;
	Vec tempv(Nx);
	Vec tempv1(Nx);
	Vec e(Nx);
	Vec b(Nx);

	Vec w(Nx);
		
	double *minx;	
	double *ytemp;
	double *xtemp;

	ytemp=new double[Nx];
	for (i=0;i<Nx;++i) {
		ytemp[i]=correspondingTarget[i][whichCoor];
	}
	Vec yvec(ytemp,Nx);

	xtemp=new double[Nx];
	for (i=0;i<Nx;++i) {
		xtemp[i]=correspondingSource[i][whichCoor];
	}
	Vec x(xtemp,Nx);
		

	minx=new double[Nx];
	
	calMinxByCoor(correspondingSource,Nx,minx, whichCoor);

	for (i=0;i<Nx;++i) {
		for (j=0;j<Nx;++j) {
			h.ptr[i][j]=h_functionByCoor(correspondingSource[i],j,correspondingSource,minx,whichCoor);
		}
	}

	gcv=10;
	lastgcv=20;
	while (gcv-lastgcv>0.001 || lastgcv-gcv>0.001) 
	{
		temp1.zero();
		for (i=0;i<Nx;++i) {
			temp1.ptr[i][i]=1;
		}
		mxRealMulMat(lamda,&temp1,&temp);

		mxTrMat(&h,&ht);

		mxMatMulMat(&ht,&h,&temp1);

		mxMatAddMat(&temp1,&temp,&a);

		//b=ht*y
		//a=ht*h+lamda*I
		
		if (normalRBF) {
			mxMatMulVec(&ht,&yvec,&b);
		}
		else {
			vecSub(&yvec,&x,&tempv);
			mxMatMulVec(&ht,&tempv,&b);
		}

		//aw=b
		//mxSolve(&a,&b,&w);

		////////////////////////////////////////////////////////////////////
		//calculate gcv and lamda
		
		//a^-1
		mxInverse(&a,&at1);

		mxMatMulVec(&at1, &b, &w);
		//a^-2
		mxMatMulMat(&at1,&at1,&temp);


		//lamda*a^-2
		mxRealMulMat(lamda,&temp,&temp1);

		//a^-1 - lamda* a^-2  
		mxMatSubMat(&at1,&temp1,&temp);
		ng=traceF(&temp);
		yg=((double)Nx)-lamda*traceF(&at1);
		mxMatMulVec(&h,&w,&tempv1);
		subVec(&yvec,&tempv1,&tempv);
		
		error=ndMult(&tempv,&tempv);
		lastgcv=gcv;
		gcv=Nx*error/((Nx-yg)*(Nx-yg));
		
		mxMatMulVec(&at1,&w,&tempv);
		ww=ndMult(&w,&tempv);

		lamda=ng*error/(Nx-yg)/ww;

		//printf("Error: %f  ----    Lamda: %f \n",gcv,lamda);
		

		mxMatMulVec(&h,&w,&tempv1);
		subVec(&yvec,&tempv1,&tempv);

		error=ndMult(&tempv,&tempv);

		//printf("Error: %f \n",error);		
	}
	
	for (i=0;i<NSource;++i) {
		target[i][whichCoor]=f_functionByCoor(Nx,source[i],w.ptr,correspondingSource,minx,whichCoor, normalRBF);
		//printf("(%f,%f,%f) ---> %f\n",source[i][0],source[i][1],source[i][2],target[i][whichCoor]);
	}
	
	delete allocator;
}

#define EPSILON 0.05

bool insideTriangle(double *x,double *x1,double *x2,double *x3)
{	
	double a[3],b[3],c[3];
	double la,lb,lc;
	double ca,cb,cc;
	double sa,sb,sc,sum;
	int i;

	for (i=0;i<3;i++) {
		a[i]=x1[i]-x[i];
		b[i]=x2[i]-x[i];
		c[i]=x3[i]-x[i];
	}

	la=vecLen(a);
	lb=vecLen(b);
	lc=vecLen(c);

	ca=noDirectionMult(a,b)/la/lb;
	cb=noDirectionMult(b,c)/lb/lc;
	cc=noDirectionMult(c,a)/lc/la;

	/*sa=sqrt(1-ca*ca);
	sb=sqrt(1-cb*cb);
	sc=sqrt(1-cc*cc);

	sum=ca*cb*cc-sa*sb*cc-sa*cb*sc-ca*sb*sc;

	if (sum>=1-EPSILON && sum<=1+EPSILON) {
		return true;
	}*/

	sum=acos(ca)+acos(cb)+acos(cc);

	if (sum>=2*PI-EPSILON && sum<=2*PI+EPSILON) {
		return true;
	}
		
	return false;
	

	/*

	Mat a(3,3);
	Vec b(x,3);
	Vec xx(3);
	int i;
	double det;

	for (i=0;i<3;i++) {
		a.ptr[i][0]=x1[i];
		a.ptr[i][1]=x2[i];
		a.ptr[i][2]=x3[i];
	}

	det=a.ptr[0][0]*(a.ptr[1][1]*a.ptr[2][2]-a.ptr[1][2]*a.ptr[2][1])-
		a.ptr[1][0]*(a.ptr[0][1]*a.ptr[2][2]-a.ptr[0][2]*a.ptr[2][1])+
		a.ptr[2][0]*(a.ptr[0][1]*a.ptr[1][2]-a.ptr[0][2]*a.ptr[1][1]);

	if (det<EPSILON && det>-EPSILON) {
		return false;
	}

	mxSolve(&a,&b,&xx);

	for (i=0;i<3;i++) {
		if (xx.ptr[i]<-EPSILON || xx.ptr[i]>1.0f+EPSILON ) {
			return false;
		}
	}
	return true;
	*/
}
