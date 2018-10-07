// Simulator.cpp: 定义控制台应用程序的入口点。
//

#include"stdafx.h"
#include"Pcenter.h"
using namespace std;

int main()
{
	string instanceName = "u1817.tsp";
	testInstance(instanceName);
    return 0;
}
void testInstance(string name) {
	 string instancePath = INSTANCE_DIR + name;
	 PcenterSolver *solver = new PcenterSolver(instancePath);
	 solver->randseed = (int)time(NULL);
	 srand(solver->randseed);
	 PcenterSolver_solve(solver);
}

int PcenterSolver_solve(PcenterSolver *solver) {
	init_solution(solver);
	int *centerNodes = solver->solution->centerNodes;
	float **distance = solver->distance;
	int node_num = solver->node_num;
	
	return 1;
}

Move find_move(PcenterSolver *solver) {
	int bft_node[K], bft_num = 0;//备选的普通节点
	Solution *solution = solver->solution;
	int *indexInCenterNodeArray = solution->indexInCenterNodeArray;
	int *centerNodes = solution->centerNodes;
	int node_num = solver->node_num, longest_edge_node = solution->longest_edge_node;
	float **distance = solver->distance, logest_edge_length = solution->longest_edge_length;
	for (int i = 0; i < node_num &&bft_num<K; i++) {//TODO:这里需要考虑一下怎么选K个
		if (indexInCenterNodeArray[i] == -1 && distance[longest_edge_node][i] < logest_edge_length) {//从非服务站点中寻找
			bft_node[bft_num++] = i;
		}
	}
	for (int i = 0; i < K; i++) {
		int o_node = bft_node[i];

		for (int j = 0; j < P_NUM; j++) {
			int center_node = centerNodes[j];

		}
	}
}

bool decrease_compare(const DisNode &a, const DisNode &b) {
	return (a.distance > b.distance);
}
bool increase_compare(const DisNode &a, const DisNode &b) {
	return (a.distance > b.distance);
}
void init_solution(PcenterSolver *solver) {
	int node_num = solver->node_num;
	Solution *solution = solver->solution;
	vector <DisNode> disNode;
	float **distance = solver->distance;
	int node = rand() % node_num;//首先随机选一个node作为Center
	disNode.reserve(node_num);
	while (solution->centerNum < P_NUM) {//选择某些node为Center，复杂度为(P_NUM*(n+KlogK))
		solution->addCenterNode(node);
		for (int i = 0; i < node_num; i++) {
			if (solution->indexInCenterNodeArray[i] != -1)//已经被选为中心的不再选择
				disNode[i].distance = -1;
			else
				disNode[i].distance = distance[node][i];
			disNode[i].node = i;
		}
		partial_sort(disNode.begin(), disNode.begin() + K, disNode.begin() + node_num, decrease_compare);
		node = disNode[rand() % K].node;//从前K个距离最远的里面随机挑一个node
	}

	int *centerNodes = solver->solution->centerNodes, max_num,max_node;
	int(*F)[2] = solver->F;
	float(*D)[2] = solver->D, max = -1;
	disNode.reserve(P_NUM);
	for (int i = 0; i < node_num; i++) {//初始化F表和D表
		for (int j = 0; j < P_NUM; j++) {
			disNode[j].distance = distance[i][centerNodes[j]];
			disNode[j].node = centerNodes[j];
		}
		partial_sort(disNode.begin(), disNode.begin() + 2, disNode.begin() + P_NUM, increase_compare);//找出最小的两条
		for (int j = 0; j < 2; j++) {
			F[i][j] = disNode[j].node;
			D[i][j] = disNode[j].distance;
		}
		if (D[i][0] > max) {//D[i][0]为i到其服务点的距离,F[i][0]为i的服务点
			max_num = 1;
			max = D[i][0];
			max_node = i;
		}
		else if (D[i][0] == max) {
			if (rand() % max_num < 1) {//随机从最长服务边中选一条
				max_node = i;
			}
			max_num++;
		}
	}
	solver->solution->longest_edge_length = max;
	solver->solution->longest_edge_node = max_node;
}

