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
#define MAX_ITERATIONS 10000
#define MAX_NO_IMPROVE_ITERATIONS 3000
typedef struct Coord{
	float x, y;
}Coord;
static float calculateDistance(Coord a, Coord b) {
	return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}
typedef struct Move {
	int o_node, center_node;
}Move;
typedef struct Solution {
	int *centerNodes;//用来存储centerNodes
	int *best_centerNodes; //最好的解对应的centerNodes的分布
	int *indexInCenterNodeArray;
	int centerNum;
	float f,recovery_f,best_f;
	int longest_edge_node;
	Solution(int node_num) {
		this->centerNodes = (int *)malloc(sizeof(int)*node_num);
		this->best_centerNodes = (int *)malloc(sizeof(int)*node_num);
		this->indexInCenterNodeArray = (int *)malloc(sizeof(int)*node_num);
		memset(this->indexInCenterNodeArray, -1, sizeof(int)*node_num);
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
	void copyCenterNodesTobest() {
		for (int i = 0; i < centerNum; i++){
			best_centerNodes[i] = centerNodes[i];
		}
	}
	void print_solution() {
		std::cout << "The length of longest service edge is " << best_f << std::endl;
		for (int i = 0; i < centerNum; i++) {
			std::cout << best_centerNodes[i] << " ";
			if (i % 10 == 0)
				std::cout << std::endl;
		}
	}
}Solution;
typedef struct NodesOfCN {
	int *nodesOfCentroid[P_NUM];
	int *index[P_NUM];
	int num[P_NUM];
	NodesOfCN(int node_num) {
		for (int i = 0; i < P_NUM; i++) {
			nodesOfCentroid[i] = new int[node_num];
			index[i] = new int[node_num];
			memset(index[i], -1, sizeof(int)*node_num);
			num[i] = 0;
		}
	}
	void add_o_node_to_centerNode(int o_node,int centerNode) {
		index[centerNode][o_node] = num[centerNode];
		nodesOfCentroid[centerNode][num[centerNode]++] = o_node;
	}
	void delete_o_nodeOfCenterNode(int o_node, int centerNode) {
		int i = index[centerNode][o_node];
		nodesOfCentroid[centerNode][i] = nodesOfCentroid[centerNode][--num[centerNode]];
		index[centerNode][nodesOfCentroid[centerNode][i]] = i;
		index[centerNode][o_node] = -1;
	}
	void delete_centerNode(int centerNode) {
		num[centerNode] = 0;
	}
}NodesOfCN;
class PcenterSolver {
public:
	int node_num;
	std::vector<Coord> coord;
	float **distance;
	Solution *solution;
	NodesOfCN *nodesOfCenter;
	int iter;
	int randseed;
	int **tabu_list;
	int (*F)[2];
	float (*D)[2];
	void readTspFile(std::string filename);
	PcenterSolver(std::string filename) {
		readTspFile(filename);//读取坐标信息
		this->node_num = this->coord.size();
		int len = this->node_num;
		for (int i = 0; i < len; i++) {
			this->distance = (float **)malloc(sizeof(float*)*len);
			this->tabu_list = (int **)malloc(sizeof(int *)*len);
		}
		for (int i = 0; i < len; i++) {
			this->distance[i] = (float *)malloc(sizeof(float)*len);
			this->tabu_list[i] = (int *)calloc(len, sizeof(int));
		}
		for (int i = 0; i < len; i++) {//计算距离
			for (int j = 0; j < len; j++) {
				this->distance[i][j] = calculateDistance(this->coord[i], this->coord[j]);
			}
		}
		this->solution = new Solution(this->node_num);
		this->nodesOfCenter = new NodesOfCN(this->node_num);
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
Move *find_move(PcenterSolver *solver, int iter);
void make_move(Move *move, PcenterSolver *solver, int iter);
void check_solution(PcenterSolver *solver);