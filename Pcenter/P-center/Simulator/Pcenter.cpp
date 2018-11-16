#include"stdafx.h"
#include"Pcenter.h"


void PcenterSolver::readTspFile(std::string filename) {
	std::ifstream readFile(filename);
	std::string str;
	float x, y;
	int cnt = 0, index;
	while (std::getline(readFile, str)) {
		if (cnt && str != "EOF") {
			std::stringstream ss(str);
			ss >> index >> x >> y;
			Coord coord = { x,y };
			this->coord.push_back(coord);
		}
		cnt++;
	}
	int len = this->node_num;
	for (int i = 0; i < len; i++) {//¼ÆËã¾àÀë
		for (int j = 0; j < len; j++) {
			this->distance[i][j] = calculateDistance(this->coord[i], this->coord[j]);
		}
	}
}
void PcenterSolver::readTxtFile(std::string filename) {
	std::ifstream readFile(filename);
	std::string str;
	int cnt = 0, u, v, dis, len = this->node_num;
	for (int i = 0; i < len; i++) {//¼ÆËã¾àÀë
		for (int j = 0; j < len; j++) {
			if (i == j)
				distance[i][j] = 0;
			else
				distance[i][j] = FLOAT_MAX;
		}
	}
	while (std::getline(readFile, str)) {
		if (cnt && str != "EOF") {
			std::stringstream ss(str);
			ss >> u >> v >> dis;
			distance[u-1][v-1] = dis;
			distance[v-1][u-1] = dis;
		}
		cnt++;
	}
	for (int k = 0; k < len; k++) {
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < len; j++) {
				distance[i][j] = std::min(distance[i][j], distance[i][k] + distance[k][j]);
			}
		}
	}
}