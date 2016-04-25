#pragma once
#include "stdafx.h"
#include "Vertex.h"

class CModel
{
public:
	vector<CVertex> vertices;
	vector<CMesh> meshes;

	CModel();
	~CModel();
	CModel(const CModel &m){
		vertices = m.vertices;
		meshes = m.meshes;
	}

	void loadFromObjFile(const string &fileName){
		ifstream file(fileName);
		string line;
		vertices.clear();
		meshes.clear();

		while (getline(file, line)){
			if (line.find("v ") == 0){
				std::istringstream iss(line.substr(2));
				CVertex v;
				iss >> v.x >> v.y >> v.z;
				vertices.push_back(v);
			}
			else if (line.find("f ") == 0){
				CMesh m;
				line = line.substr(2);
				std::istringstream iss(line);
				int b;
				char c;
				iss >> m.a >> c;
				iss >> b >> c;
				iss >> b;
				iss >> m.b >> c >> b >> c >> b;
				iss >> m.c >> c >> b >> c >> b;
				meshes.push_back(m);
			}
		}
	}

	void writeToObjFile(const string &fileName) {
		ofstream file(fileName);
		for (int i = 0; i < vertices.size(); i++) {
			file << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;
		}

		for (int i = 0; i < meshes.size(); i++) {
			file << "f " << meshes[i].a << " " << meshes[i].b << " " << meshes[i].c << endl;
		}
	}
};
