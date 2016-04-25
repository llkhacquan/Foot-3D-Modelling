#include "stdafx.h"
#include "tiny_obj_loader.h"
#include <vector>
#include "Vertex.h"


using namespace std;

static bool TestStreamLoadObj()
{
	std::cout << "Stream Loading " << std::endl;

	std::stringstream objStream;
	objStream
		<< "mtllib cube.mtl\n"
		"\n"
		"v 0.000000 2.000000 2.000000\n"
		"v 0.000000 0.000000 2.000000\n"
		"v 2.000000 0.000000 2.000000\n"
		"v 2.000000 2.000000 2.000000\n"
		"v 0.000000 2.000000 0.000000\n"
		"v 0.000000 0.000000 0.000000\n"
		"v 2.000000 0.000000 0.000000\n"
		"v 2.000000 2.000000 0.000000\n"
		"# 8 vertices\n"
		"\n"
		"g front cube\n"
		"usemtl white\n"
		"f 1 2 3 4\n"
		"g back cube\n"
		"# expects white material\n"
		"f 8 7 6 5\n"
		"g right cube\n"
		"usemtl red\n"
		"f 4 3 7 8\n"
		"g top cube\n"
		"usemtl white\n"
		"f 5 1 4 8\n"
		"g left cube\n"
		"usemtl green\n"
		"f 5 6 2 1\n"
		"g bottom cube\n"
		"usemtl white\n"
		"f 2 6 7 3\n"
		"# 6 elements";

	std::string matStream(
		"newmtl white\n"
		"Ka 0 0 0\n"
		"Kd 1 1 1\n"
		"Ks 0 0 0\n"
		"\n"
		"newmtl red\n"
		"Ka 0 0 0\n"
		"Kd 1 0 0\n"
		"Ks 0 0 0\n"
		"\n"
		"newmtl green\n"
		"Ka 0 0 0\n"
		"Kd 0 1 0\n"
		"Ks 0 0 0\n"
		"\n"
		"newmtl blue\n"
		"Ka 0 0 0\n"
		"Kd 0 0 1\n"
		"Ks 0 0 0\n"
		"\n"
		"newmtl light\n"
		"Ka 20 20 20\n"
		"Kd 1 1 1\n"
		"Ks 0 0 0");

	using namespace tinyobj;
	class MaterialStringStreamReader :
		public MaterialReader
	{
	public:
		MaterialStringStreamReader(const std::string& matSStream) : m_matSStream(matSStream) {}
		virtual ~MaterialStringStreamReader() {}
		virtual bool operator() (
			const std::string& matId,
			std::vector<material_t>& materials,
			std::map<std::string, int>& matMap,
			std::string& err)
		{
			(void)matId;
			(void)err;
			LoadMtl(matMap, materials, m_matSStream);
			return true;
		}

	private:
		std::stringstream m_matSStream;
	};

	MaterialStringStreamReader matSSReader(matStream);
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(shapes, materials, err, objStream, matSSReader);

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	return true;
}

void extractVerticesAndFace(string sInFile, string sOutFile){
	std::ifstream inFile(sInFile);
	std::ofstream outFile(sOutFile);
	std::string line;

	while (getline(inFile, line)){
		if (line.find("v ") == 0){
			outFile << line << endl;
		}
		else if (line.find("f ") == 0){
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

	inFile.close();
	outFile.close();
}

vector<CVertex> readVertices(string sfile){
	ifstream file(sfile);
	string line;

	vector<CVertex> vertices;
	while (getline(file, line)){
		if (line.find("v ") == 0){
			std::istringstream iss(line.substr(2));
			CVertex v;
			iss >> v.x >> v.y >> v.z;
			vertices.push_back(v);
		}
	}
	return vertices;
}

vector<pair<int, int>> jointVertice(const vector<CVertex> &vertices1, const vector<CVertex> &vertices2){
	vector<pair<int, int>> re;
	pair<int, int> p;
	for (int i = 0; i < vertices1.size(); i++){
		for (int j = 0; j < vertices2.size(); j++){
			if (vertices1[i] == vertices2[j]){
				p.first = i; p.second = j;
				re.push_back(p);
				goto lable;
			}
		}
	lable:
		continue;
	}
	return re;
}


vector<pair<int, int>> jointVertice(const string sFile1, const string sFile2, const string sFile3 = ""){
	vector<CVertex> vertices1 = readVertices(sFile1);
	vector<CVertex> vertices2 = readVertices(sFile2);

	vector<pair<int, int>> re = jointVertice(vertices1, vertices2);

	cout << "There are " << re.size() << " similar vertices" << endl;

	if (sFile3.length() > 0){
		ofstream outFile(sFile3);
		for each (pair<int, int> p in re)
		{
			outFile << p.first << " " << p.second << endl;
		}
	}

	return re;
}

int detectPoint(const string sFile1, const string sFile2){
	vector<CVertex> vertices1 = readVertices(sFile1);
	vector<CVertex> vertices2 = readVertices(sFile2);
	assert(vertices1.size() == vertices2.size());
	int found = -1;
	for (int i = 0; i < vertices1.size(); i++){
		for (int j = 0; j < vertices2.size(); j++){
			if (vertices1[i] == vertices2[j]){
				vertices2.erase(vertices2.begin() + j);
				goto label;
			}
		}
		return i;

	label:
		continue;
	}
	return -1;
}


vector<int> detectPoints(const string sFile1, const string sFile2){
	vector<CVertex> vertices1 = readVertices(sFile1);
	vector<CVertex> vertices2 = readVertices(sFile2);
	vector<int> re;
	assert(vertices1.size() == vertices2.size());
	int found = -1;
	for (int i = 0; i < vertices1.size(); i++){
		for (int j = 0; j < vertices2.size(); j++){
			if (vertices1[i] == vertices2[j]){
				goto label;
			}
		}
		re.push_back(i);

	label:
		continue;
	}
	return re;
}

float distance(vector<CVertex> v, int p1, int p2){
	assert(v.size() > p1 && v.size()>p2);
	return sqrt((v[p1].x - v[p2].x)*(v[p1].x - v[p2].x) + (v[p1].y - v[p2].y)*(v[p1].y - v[p2].y) + (v[p1].z - v[p2].z)*(v[p1].z - v[p2].z));
}