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
	
}