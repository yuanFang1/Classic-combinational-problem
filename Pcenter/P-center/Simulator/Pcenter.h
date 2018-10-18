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
#define INSTANCE_LIST "instance.txt"
#define MAX_PATH_LEN 256
#define K 5
#define MAX_ITERATIONS 100000
#define MAX_NO_IMPROVE_ITERATIONS 30000
#define LOG_ON 1
//#define LOG_MOVE 1
//#define TABU_TEST
#define SEPARATOR "================"
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
	float f,new_f,best_f;
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
	void print_solution(int print_center) {
		std::cout << "The length of longest service edge is " << best_f << std::endl;
		if (print_center) {
			for (int i = 0; i < centerNum; i++) {
				std::cout << best_centerNodes[i] << " ";
				if (i != 0 && i % 10 == 0)
					std::cout << std::endl;
			}
		}
		
	}
}Solution;
typedef struct NodesOfCN {
	int **nodesOfCentroid;
	int **index;
	int *num;
	NodesOfCN(int node_num,int p_num) {
		nodesOfCentroid = new int *[p_num];
		index = new int*[p_num];
		num = new int[p_num];
		for (int i = 0; i < p_num; i++) {
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
	int p_num;
	float true_best_f;
	std::vector<Coord> coord;
	float **distance;
	Solution *solution;
	NodesOfCN *nodesOfCenter;
	int iter;
	int randseed;
	int **tabu_list;
	int (*F)[2];
	float (*D)[2];
	void readTxtFile(std::string filename);
	void readTspFile(std::string filename);
	PcenterSolver(std::string filename, int node_num,int p_num, int true_best_f) {
		this->node_num = node_num;
		this->p_num = p_num;
		this->true_best_f = true_best_f;
		
		this->tabu_list = (int **)malloc(sizeof(int *)*node_num);
		this->distance = (float **)malloc(sizeof(float*)*node_num);
		for (int i = 0; i < node_num; i++) {
			this->tabu_list[i] = (int *)calloc(node_num, sizeof(int));
			this->distance[i] = (float *)malloc(sizeof(float)*node_num);
		}
		if (filename.find(".txt") != std::string::npos) {
			readTxtFile(filename);
		}
		else {
			readTspFile(filename);//读取坐标信息

		}
		this->solution = new Solution(this->node_num);
		this->nodesOfCenter = new NodesOfCN(this->node_num,this->p_num);
		this->F = new int[this->node_num][2];
		this->D = new float[this->node_num][2];
	}
};
typedef struct DisNode {
	float distance;
	int node;
}DisNode;
void testInstance(std::string name, int node_num, int p_num, int true_best_f);
int PcenterSolver_solve(PcenterSolver *solver);
void init_solution(PcenterSolver *solver);
Move *find_move(PcenterSolver *solver, int iter);
void make_move(Move *move, PcenterSolver *solver, int iter);
void check_current_solution(PcenterSolver *solver);
void check_end_solution(PcenterSolver * solver);
void check_D(PcenterSolver *solver);