#include "stdafx.h"
#include "Model.h"
#include "Feature.h"
#include "morph.h"

#define MAXMAXX 1000000
#define MINMINX -1000000
#define MAXMAXY 1000000
#define MINMINY -1000000
#define MAXMAXZ 1000000
#define MINMINZ -1000000

#define EPSILON 0.005

using namespace std;

const double eps = 1e-4;

const double normal_rbf_ratio = 1.0;

void morphFace(CModel &sourceHead, const vector <CFeature> &features, const bool normalRBF = true);

double calculateRatio(double** point, const int& nVertex) {
	double minY = MAXMAXY;
	double maxY = MINMINY;
	for (int i = 0; i < nVertex; ++i) {
		if (point[i][1] < minY) minY = point[i][1];
		if (point[i][1] > maxY) maxY = point[i][1];
	}
	return 1 / (maxY - minY);
}


void applyRatio(const double& ratio, double** point, const int& nVertex) {
	for (int i = 0; i < nVertex; ++i) {
		point[i][0] *= ratio;
		point[i][1] *= ratio;
		point[i][2] *= ratio;
	}
}

void convertFace(CModel &sourceModel, const vector<CFeature> &features){
	CModel originalModel;

	double** source_normal_rbf_true;
	int nSourceVertex = sourceModel.vertices.size();
	source_normal_rbf_true = new double*[nSourceVertex];

	sourceModel = originalModel;
	morphFace(sourceModel, features, true);
	for (int i = 0; i < nSourceVertex; ++i) {
		source_normal_rbf_true[i] = new double[3];
		source_normal_rbf_true[i][0] = sourceModel.vertices[i].x;
		source_normal_rbf_true[i][1] = sourceModel.vertices[i].y;
		source_normal_rbf_true[i][2] = sourceModel.vertices[i].z;
	}

	double** source_normal_rbf_false;
	source_normal_rbf_false = new double*[nSourceVertex];

	sourceModel = originalModel;
	morphFace(sourceModel, features, false);

	for (int i = 0; i < nSourceVertex; ++i) {
		source_normal_rbf_false[i] = new double[3];
		source_normal_rbf_false[i][0] = sourceModel.vertices[i].x;
		source_normal_rbf_false[i][1] = sourceModel.vertices[i].y;
		source_normal_rbf_false[i][2] = sourceModel.vertices[i].z;
	}

	for (int i = 0; i < nSourceVertex; ++i) {
		sourceModel.vertices[i].x = source_normal_rbf_true[i][0] * normal_rbf_ratio + source_normal_rbf_false[i][0] * (1 - normal_rbf_ratio);
		sourceModel.vertices[i].y = source_normal_rbf_true[i][1] * normal_rbf_ratio + source_normal_rbf_false[i][1] * (1 - normal_rbf_ratio);
		sourceModel.vertices[i].z = source_normal_rbf_true[i][2] * normal_rbf_ratio + source_normal_rbf_false[i][2] * (1 - normal_rbf_ratio);
	}

	for (int i = 0; i < nSourceVertex; ++i) {
		delete[] source_normal_rbf_true[i];
		delete[] source_normal_rbf_false[i];
	}
	delete[] source_normal_rbf_true;
	delete[] source_normal_rbf_false;
}

void calculateTheBoundingBoxCenter(double** point, const int& nVertex, double& xCenter, double& yCenter, double& zCenter) {
	double minX = MAXMAXX;
	double maxX = MINMINX;
	double minY = MAXMAXY;
	double maxY = MINMINY;
	double minZ = MAXMAXZ;
	double maxZ = MINMINZ;
	for (int i = 0; i < nVertex; ++i) {
		if (point[i][0] < minX) minX = point[i][0];
		if (point[i][0] > maxX) maxX = point[i][0];
		if (point[i][1] < minY) minY = point[i][1];
		if (point[i][1] > maxY) maxY = point[i][1];
		if (point[i][2] < minZ) minZ = point[i][2];
		if (point[i][2] > maxZ) maxZ = point[i][2];
	}
	xCenter = (minX + maxX) / 2;
	yCenter = (minY + maxY) / 2;
	zCenter = (minZ + maxZ) / 2;
}

void applyTranslation(const double dx, const double dy, const double dz,
	double** targetFeatures, const int nFeatures) {
	for (int i = 0; i < nFeatures; ++i) {
		targetFeatures[i][0] += dx;
		targetFeatures[i][1] += dy;
		targetFeatures[i][2] += dz;
	}
}

void morphFace(CModel &sourceHead, const vector <CFeature> &features, const bool normalRBF){
	double** sourceFeatures;
	double** targetFeatures;
	int nFeatures = features.size();

	sourceFeatures = new double*[nFeatures];
	targetFeatures = new double*[nFeatures];

	for (int i = 0; i < nFeatures; ++i) {
		sourceFeatures[i] = new double[3];
		targetFeatures[i] = new double[3];
	}

	for (int i = 0; i < nFeatures; ++i) {
		// TODO MMeshVertex sourceVertex = features->getLandmarkAt(i).getVertex();
		CVertex sourceVertex = features[i].source;
		sourceFeatures[i][0] = sourceVertex.x;
		sourceFeatures[i][1] = sourceVertex.y;
		sourceFeatures[i][2] = sourceVertex.z;

		// TODO MMeshVertex targetVertex = features->getNeedleAt(i).getEndPoint();
		CVertex targetVertex = features[i].target;
		targetFeatures[i][0] = targetVertex.x;
		targetFeatures[i][1] = targetVertex.y;
		targetFeatures[i][2] = targetVertex.z;
	}

	double **source;
	double **target;

	int nSourceVertex = sourceHead.vertices.size();
	source = new double*[nSourceVertex];
	target = new double*[nSourceVertex];

	for (int i = 0; i < nSourceVertex; ++i) {
		source[i] = new double[3];
		target[i] = new double[3];
	}

	if (nFeatures > 0)
		for (int i = 0; i < nSourceVertex; ++i) {
			source[i][0] = sourceHead.vertices[i].x;
			source[i][1] = sourceHead.vertices[i].y;
			source[i][2] = sourceHead.vertices[i].z;
		}

	double sourceRatio = calculateRatio(source, nSourceVertex);
	applyRatio(sourceRatio, source, nSourceVertex);
	applyRatio(sourceRatio, sourceFeatures, nFeatures);

	double targetRatio = calculateRatio(targetFeatures, nFeatures);
	applyRatio(targetRatio, targetFeatures, nFeatures);

	double xSourceCenter, ySourceCenter, zSourceCenter;
	double xTargetCenter, yTargetCenter, zTargetCenter;
	calculateTheBoundingBoxCenter(source, nSourceVertex, xSourceCenter, ySourceCenter, zSourceCenter);
	calculateTheBoundingBoxCenter(targetFeatures, nFeatures, xTargetCenter, yTargetCenter, zTargetCenter);
	double dx = xSourceCenter - xTargetCenter;
	double dy = ySourceCenter - yTargetCenter;
	double dz = zSourceCenter - zTargetCenter;

	applyTranslation(dx, dy, dz, targetFeatures, nFeatures);

	morph(sourceFeatures, targetFeatures, source, target, nFeatures, sourceHead.vertices.size(), 0, normalRBF);
	morph(sourceFeatures, targetFeatures, source, target, nFeatures, sourceHead.vertices.size(), 1, normalRBF);
	morph(sourceFeatures, targetFeatures, source, target, nFeatures, sourceHead.vertices.size(), 2, normalRBF);

	applyTranslation(-dx, -dy, -dz, target, nSourceVertex);
	applyRatio(1 / targetRatio, target, nSourceVertex);


	for (int i = 0; i < nSourceVertex; ++i) {
		sourceHead.vertices[i].x = target[i][0];
		sourceHead.vertices[i].y = target[i][1];
		sourceHead.vertices[i].z = target[i][2];
	}

	for (int i = 0; i < nFeatures; ++i) {
		delete[] sourceFeatures[i];
		delete[] targetFeatures[i];
	}
	delete[] sourceFeatures;
	delete[] targetFeatures;

	for (int i = 0; i < nSourceVertex; ++i) {
		delete[] source[i];
		delete[] target[i];
	}
	delete[] source;
	delete[] target;
}

