#pragma once
#include"stdafx.h"
#include<string>
#include<sstream>
#include<iostream>
#include<algorithm>
#include<math.h>
#include<time.h>
#include<fstream>
#define INSTANCE_DIR  "Instance\\"
#define MAX_PATH_LEN 256
#define P_NUM 50
#define K 5
static float calculateDistance(Coord a, Coord b) {
	return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}
typedef struct Coord{
	float x, y;
}Coord;
typedef struct Move {
	int o_node, center_node;
}Move;
typedef struct Solution {
	int *centerNodes;//用来存储centerNodes
	int *indexInCenterNodeArray;
	int centerNum;
	float longest_edge_length;
	int longest_edge_node;
	Solution(int node_num) {
		this->centerNodes = (int *)malloc(sizeof(int)*node_num);
		memset(this->centerNodes, -1, sizeof(int)*node_num);
		this->indexInCenterNodeArray = (int *)malloc(sizeof(int)*node_num);
		this->centerNum = 0;
	}
	void addCenterNode(int node) {
		indexInCenterNodeArray[node] = centerNum;
		centerNodes[centerNum++] = node;
	}
	void deleteCenterNode(int node) {
		int index = indexInCenterNodeArray[node];
		centerNodes[index] = centerNodes[--centerNum];
		indexInCenterNodeArray[centerNodes[index]] = index;
		indexInCenterNodeArray[node] = -1;
	}
}Solution;
class PcenterSolver {
public:
	int node_num;
	std::vector<Coord> coord;
	float **distance;
	Solution *solution;
	int randseed;
	int (*F)[2];
	float (*D)[2];
	void readTspFile(std::string filename);
	PcenterSolver(std::string filename) {
		readTspFile(filename);//读取坐标信息
		this->node_num = this->coord.size();
		int len = this->node_num;
		for (int i = 0; i < len; i++) {//计算距离
			this->distance = (float **)malloc(sizeof(float*)*len);
		}
		for (int i = 0; i < len; i++) {
			this->distance[i] = (float *)malloc(sizeof(float)*len);
		}
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < len; j++) {
				this->distance[i][j] = calculateDistance(this->coord[i], this->coord[j]);
			}
		}
		this->solution = new Solution(this->node_num);
		this->F = new int[this->node_num][2];
		this->D = new float[this->node_num][2];
	}
};
typedef struct DisNode {
	float distance;
	int node;
}DisNode;
void testInstance(std::string name);
int PcenterSolver_solve(PcenterSolver *solver);
void init_solution(PcenterSolver *solver);
Move find_move(PcenterSolver *solver);