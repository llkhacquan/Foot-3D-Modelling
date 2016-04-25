#include "stdafx.h"
#include "general_function.h"
#include "Model.h"
#include "Feature.h"
#include "algo.h"


using namespace std;

// we must prepare 3 file obj as above
const string ORIGINAL_FILE = "data/feet.obj";
const string BACK_FILE = "data/back.obj";
const string FRONT_FILE = "data/front.obj";
const string TOP_FILE = "data/top.obj";
const string BOTTOM_FILE = "data/bottom.obj";


// text file
const string BACK_VERTICES = "data/back.txt"; // contains the vertices index of back-part in original file;
const string FRONT_VERTICES = "data/front.txt"; // contains the vertices index of front-part in original file;

void process();

CVertex featureVertices[7];
vector<CVertex> v_origin;
vector<CVertex> v_front;
vector<CVertex> v_back;
float d[8];
int point[8][3];

int main(int argc, char **argv)
{
	//extractVerticesAndFace("feet_hands.obj", "feet_hands2.obj");

	// jointVertice(ORIGINAL_FILE, BACK_FILE, BACK_VERTICES); // run this method to calculate back_vertices file
	// jointVertice(ORIGINAL_FILE, FRONT_FILE, FRONT_VERTICES); // run this method to calculate front_vertices file

	// detect index of a key point in original file:
	//    open the original file, move the key point (only 1 point), save the file as "data/ankle.obj" for example
	// cout << "The line 4 is (" << detectPoint(ORIGINAL_FILE, "data/41.obj") << " " << detectPoint(ORIGINAL_FILE, "data/42.obj") << endl; //(429, 49)

	// 
	process();

	CModel model;
	model.loadFromObjFile("data/feet.obj");
	vector<CFeature> features;
	for (int i = 1; i <= 7; i++) {
		for (int j = 1; j <= 2; j++) {
			if (i == 3)
				continue;
			CFeature f;
			f.source = model.vertices[point[i][j]];
			f.target = v_origin[point[i][j]];
			features.push_back(f);
		}
	}
	morphFace(model, features, true);

	model.writeToObjFile("data/output2.obj");

	return 0;
}

void scale_4_x() {
	point[4][1] = detectPoint(ORIGINAL_FILE, "data/41.obj");
	point[4][2] = detectPoint(ORIGINAL_FILE, "data/42.obj");

	float d4 = abs(v_origin[point[4][1]].x - v_origin[point[4][2]].x);
	float ratio = d[4] / d4;
	for (int i = 0; i < v_origin.size(); i++) {
		v_origin[i].x *= ratio;
	}
}

void scale_1_z() {
	point[1][1] = detectPoint(ORIGINAL_FILE, "data/11.obj");
	point[1][2] = detectPoint(ORIGINAL_FILE, "data/12.obj");

	float d1 = abs(v_origin[point[1][1]].z - v_origin[point[1][2]].z);
	float ratio = d[1] / d1;
	for (int i = 0; i < v_origin.size(); i++) {
		v_origin[i].z *= ratio;
	}
}

void scale_2_x_font_only() {
	point[2][1] = detectPoint(ORIGINAL_FILE, "data/21.obj");
	point[2][2] = detectPoint(ORIGINAL_FILE, "data/22.obj");
	float d2 = sqrt((v_origin[point[2][1]].x - v_origin[point[2][2]].x)*(v_origin[point[2][1]].x - v_origin[point[2][2]].x) +
		(v_origin[point[2][1]].z - v_origin[point[2][2]].z)*(v_origin[point[2][1]].z - v_origin[point[2][2]].z));
	float ratio = d[2] / d2;

	// joint save the index in original file of points which are the joint of front and back
	vector<int> joint;
	vector<pair<int, int>> jointOfFrontAndBack = jointVertice(FRONT_FILE, BACK_FILE);
	vector<pair<int, int>> jointOfOiginAndFront = jointVertice(ORIGINAL_FILE, FRONT_FILE);
	for (int i = 0; i < jointOfFrontAndBack.size(); i++) {
		for (int j = 0; j < jointOfOiginAndFront.size(); j++) {
			if (jointOfFrontAndBack[i].first == jointOfOiginAndFront[j].second) {
				joint.push_back(jointOfOiginAndFront[j].first);
				goto label;
			}
		label:
			continue;
		}
	}

	// calculate origin vertex: the center of joint points
	CVertex origin;
	for each (int i in joint) {
		origin = origin + v_origin[i];
	}
	origin = origin * (1 / joint.size());

	// scale every point in jointOfOiginAndFront, except the joint
	for each (pair<int, int> p in jointOfOiginAndFront) {
		for (int i = 0; i < joint.size(); i++) {
			if (p.first == joint[i])
				goto nextP;
		}
		v_origin[p.first].x = origin.x + (v_origin[p.first].x - origin.x) * ratio;
	nextP:
		continue;
	}
}

void scale_7_y() {
	point[7][1] = detectPoint(ORIGINAL_FILE, "data/71.obj");
	point[7][2] = detectPoint(ORIGINAL_FILE, "data/72.obj");

	float d7 = abs(v_origin[point[7][1]].y - v_origin[point[7][2]].y);
	float ratio = d[7] / d7;

	for (int i = 0; i < v_origin.size(); i++) {
		v_origin[i].y *= ratio;
	}
}

void translate_6_y() {
	point[6][1] = detectPoint(ORIGINAL_FILE, "data/61.obj");
	point[6][2] = detectPoint(ORIGINAL_FILE, "data/62.obj");

	float d6 = abs(v_origin[point[6][1]].y - v_origin[point[6][2]].y);
	float trans = d[6] - d6;

	vector<pair<int, int>> upper = jointVertice(ORIGINAL_FILE, TOP_FILE);
	for each (auto p in upper)
	{
		v_origin[p.first].y += trans;
	}
}

void scale5_y_front_only() {
	point[5][1]= detectPoint(ORIGINAL_FILE, "data/51.obj");
	point[5][2] = detectPoint(ORIGINAL_FILE, "data/52.obj");
	float d5 = abs(v_origin[point[5][1]].y - v_origin[point[5][2]].y);
	float ratio = d[5] / d5;

	// joint save the index in original file of points which are the joint of front and back
	vector<int> joint;
	vector<pair<int, int>> jointOfFrontAndBack = jointVertice(FRONT_FILE, BACK_FILE);
	vector<pair<int, int>> jointOfOiginAndFront = jointVertice(ORIGINAL_FILE, FRONT_FILE);
	for (int i = 0; i < jointOfFrontAndBack.size(); i++) {
		for (int j = 0; j < jointOfOiginAndFront.size(); j++) {
			if (jointOfFrontAndBack[i].first == jointOfOiginAndFront[j].second) {
				joint.push_back(jointOfOiginAndFront[j].first);
				goto label;
			}
		label:
			continue;
		}
	}

	// scale every point in jointOfOiginAndFront, except the joint
	for each (pair<int, int> pa in jointOfOiginAndFront) {
		for (int i = 0; i < joint.size(); i++) {
			if (pa.first == joint[i])
				goto nextP;
		}
		v_origin[pa.first].y = v_origin[point[5][2]].y + (v_origin[pa.first].y - v_origin[point[5][2]].y)* ratio;
	nextP:
		continue;
	}
}

void writeOutput(string outputFile) {
	ofstream outFile(outputFile);
	for (int i = 0; i < v_origin.size(); i++) {
		outFile << "v " << v_origin[i].x << " " << v_origin[i].y << " " << v_origin[i].z << endl;
	}

	ifstream inFile(ORIGINAL_FILE);
	string line;
	while (getline(inFile, line)) {
		if (line.find("f ") == 0) {
			line = line.substr(2);
			std::istringstream iss(line);
			int a, b;
			char c;
			iss >> a >> c;
			iss >> b >> c;
			iss >> b;
			outFile << "f " << a << " ";
			iss >> a >> c >> b >> c >> b;
			outFile << a << " ";
			iss >> a >> c >> b >> c >> b;
			outFile << a << endl;
		}
	}

	outFile.close();
	inFile.close();
}

void process() {
	v_origin = readVertices(ORIGINAL_FILE);
	v_back = readVertices(BACK_FILE);
	v_front = readVertices(FRONT_FILE);

	ifstream inputFile("data/input.txt");
	for (int i = 1; i <= 7; i++) {
		inputFile >> d[i];
	}

	inputFile.close();

	scale_4_x();
	scale_1_z();
	scale_2_x_font_only();
	scale_7_y();
	scale5_y_front_only();
	translate_6_y();

	writeOutput("data/output.obj");
}