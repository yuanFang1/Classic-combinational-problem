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
	int iter = 1;
	int best_f = solver->solution->best_f,cnt =0;
	while (iter<MAX_ITERATIONS){
		Move *move = find_move(solver,iter);
		if (move == NULL) break;
		make_move(move, solver,iter);
		if (best_f < solver->solution->best_f) {
			cnt = 0;
		}
		else
			cnt++;
		if (cnt > MAX_NO_IMPROVE_ITERATIONS)
			break;
		iter++;
	}
	solver->solution->print_solution();
	check_solution(solver);
	return 1;
}
void copy_F_D(int(*dst_F)[2], float(*dst_D)[2], int(*src_F)[2],float(*src_D)[2],int node_num) {
	for (int i = 0; i < node_num; i++) {
		for (int j = 0; j < 2; j++) {
			dst_D[i][j] = src_D[i][j];
			dst_F[i][j] = src_F[i][j];
		}
	}
}
void addCenterNodeAndUpdateF_D(PcenterSolver *solver, int o_node) {
	Solution *solution = solver->solution;
	float **distance = solver->distance;
	int node_num = solver->node_num,max_num,max_node; 
	int(*F)[2] = solver->F;
	float(*D)[2] = solver->D,max=-1;
	for (int i = 0; i < node_num; i++) {
		if (distance[o_node][i] < D[i][0]) {//o_node成为i的服务站点
			F[i][1] = F[i][0];
			D[i][1] = F[i][0];
			F[i][0] = o_node;
			D[i][0] = distance[o_node][i];
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
		else if (distance[o_node][i] < D[i][1]) {
			F[i][1] = o_node;
			D[i][1] = distance[o_node][i];
		}
	}
	solution->recovery_f = max;
	solution->longest_edge_node = max_node;
}
void deleteCenterNodeAndUpdateF_D(PcenterSolver *solver, int node) {
	Solution *solution = solver->solution;
	int *centerNodes = solution->centerNodes;
	float **distance = solver->distance;
	int node_num = solver->node_num, max_num, min_node,max_node,max=-1;
	int(*F)[2] = solver->F;
	float(*D)[2] = solver->D, min;
	for (int i = 0; i < node_num; i++) {
		if (F[i][0] == node) {
			F[i][0] = F[i][1];
			D[i][0] = D[i][1];
			min = numeric_limits<float>::max();
			for (int j = 0; j < P_NUM; j++) {
				int centerNode = centerNodes[j];
				if (centerNode != node) {
					if (distance[i][centerNode] < min) {
						min = distance[i][centerNode];
						min_node = centerNode;
					}
				}
			}
			F[i][1] = min_node;
			D[i][1] = min;
		}
		else if (F[i][1] == node) {
			min = numeric_limits<float>::max();
			for (int j = 0; j < P_NUM; j++) {
				int centerNode = centerNodes[j];
				if (centerNode != F[i][0]) {
					if (distance[i][centerNode] < min) {
						min = distance[i][centerNode];
						min_node = centerNode;
					}
				}
			}
			F[i][1] = min_node;
			D[i][1] = min;
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
	}
}
Move *find_move(PcenterSolver *solver,int iter) {
	Move *move ,*best_move ,*tabu_move ,*best_tabu_move ;
	int bft_node[K], bft_num = 0;//备选的普通节点
	Solution *solution = solver->solution;
	int *indexInCenterNodeArray = solution->indexInCenterNodeArray;
	int *centerNodes = solution->centerNodes;
	int(*F)[2] = solver->F,(*recovery_F)[2];
	float(*D)[2] = solver->D,(*recovery_D)[2];
	int node_num = solver->node_num, longest_edge_node = solution->longest_edge_node;
	float **distance = solver->distance, f = solution->f;
	int **tabu_list = solver->tabu_list;

	memset(bft_node, -1, sizeof(int)*K);
	for (int i = 0; i < node_num &&bft_num<K; i++) {//TODO:这里需要考虑一下怎么选K个
		if (indexInCenterNodeArray[i] == -1 && distance[longest_edge_node][i] < f) {//从非服务站点中寻找
			bft_node[bft_num++] = i;
		}
	}
	if (bft_num == 0) return NULL;
	move = new Move, best_move = new Move, tabu_move = new Move, best_tabu_move = new Move;
	float best = numeric_limits<float>::max(),tabu_best = numeric_limits<float>::max();
	recovery_F = new int[node_num][2];
	recovery_D = new float[node_num][2];
	for (int i = 0; i < bft_num; i++) {
		int o_node = bft_node[i];

		copy_F_D(recovery_F, recovery_D, F, D,node_num);
		addCenterNodeAndUpdateF_D(solver, o_node);//添加o_node
		float min = numeric_limits<float>::max(),tabu_min = numeric_limits<float>::max();
		for (int j = 0; j < P_NUM; j++) {//TODO:将这里的时间复杂度降低一些
			int node1, node2;
			int center_node = centerNodes[j];//假如删除掉center_node最长服务边会变成什么
			float max = -1;
			for (int k = 0; k < node_num; k++) {
				if (F[k][0] == center_node) {
					if (D[k][1] > max) {//max为原来服务点为center_node的节点删除掉center_node之后的最长服务边
						max = D[k][1];
					}
				}
			}
			if (o_node < center_node) {
				node1 = o_node;
				node2 = center_node;
			}
			else {
				node1 = center_node;
				node2 = o_node;
			}
			if (tabu_list[node1][node2]>iter) {//说明仍然处于禁忌中
				if (max < tabu_min) {
					tabu_min = max;
					tabu_move->o_node = o_node;
					tabu_move->center_node = center_node;
				}
			}
			else {
				if (max < min) {
					min = max;
					move->o_node = o_node;//TODO:这个可以删掉
					move->center_node = center_node;
				}//TODO:相等时随机选择一个
			}
			
		}
		min = max(solution->recovery_f,min);
		if (min < best) {
			best = min;
			best_move->center_node = move->center_node;
			best_move->o_node = o_node;
		}
		if (tabu_min < tabu_best) {
			tabu_best = tabu_min;
			best_tabu_move->center_node = tabu_move->center_node;
			best_tabu_move->o_node = o_node;
		}
		copy_F_D(F, D, recovery_F, recovery_D,  node_num);
	}
	delete []recovery_D;
	delete []recovery_F;
	delete move;
	if (tabu_best < best &&tabu_best < solution->best_f) {//解禁策略
		solution->f = tabu_best;
		delete best_move;
		delete tabu_move;
		return best_tabu_move;
	}
	solution->f = best;
	delete tabu_move;
	delete best_tabu_move;
	return best_move;
}
void make_move(Move *move,PcenterSolver *solver, int iter) {
	Solution *solution = solver->solution;
	int node1, node2;
	if (solution->f < solution->best_f) {
		solution->best_f = solution->f;
		solution->copyCenterNodesTobest();
	}
	solution->deleteCenterNode(move->center_node);
	solution->addCenterNode(move->o_node);
	if (move->o_node< move->center_node) {
		node1 = move->o_node;
		node2 = move->center_node;
	}
	else {
		node1 = move->center_node;
		node2 = move->o_node;
	}
	solver->tabu_list[node1][node2] = 8+rand() % 20;
	addCenterNodeAndUpdateF_D(solver, move->o_node);
	deleteCenterNodeAndUpdateF_D(solver, move->center_node);

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
	NodesOfCN *nodesOfCenter = solver->nodesOfCenter;
	vector <DisNode> disNode;
	float **distance = solver->distance;
	int node = rand() % node_num;//首先随机选一个node作为Center
	disNode.resize(node_num);
	while (solution->centerNum < P_NUM) {//选择某些node为Center，复杂度为(P_NUM*(n+KlogK))
		solution->addCenterNode(node);
		for (int i = 0; i < node_num; i++) {
			if (solution->indexInCenterNodeArray[i] != -1)//已经被选为中心的不再选择
				disNode[i].distance = -1;
			else
				disNode[i].distance = distance[node][i];
			disNode[i].node = i;
		}
		partial_sort(disNode.begin(), disNode.begin() + K, disNode.end() , decrease_compare);
		node = disNode[rand() % K].node;//从前K个距离最远的里面随机挑一个node
	}
	solution->copyCenterNodesTobest();
	int *centerNodes = solution->centerNodes, max_num,max_node;
	int(*F)[2] = solver->F;
	float(*D)[2] = solver->D, max = -1;
	disNode.resize(P_NUM);
	for (int i = 0; i < node_num; i++) {//初始化F表和D表
		for (int j = 0; j < P_NUM; j++) {
			disNode[j].distance = distance[i][centerNodes[j]];
			disNode[j].node = centerNodes[j];
		}
		partial_sort(disNode.begin(), disNode.begin() + 2, disNode.end(), increase_compare);//找出最小的两条
		for (int j = 0; j < 2; j++) {
			F[i][j] = disNode[j].node;
			D[i][j] = disNode[j].distance;
		}
		//nodesOfCenter->add_o_node_to_centerNode(i, solution->indexInCenterNodeArray[F[i][0]]);
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
	solution->f = max;
	solution->best_f = max;
	solution->longest_edge_node = max_node;
}

void check_solution(PcenterSolver *solver) {
	Solution *solution = solver->solution;
	float **distance = solver->distance;
	if (solution->centerNum != P_NUM) {
		cout << "the center node num is error" << endl;
		return;
	}
	float max = -1;
	for (int i = 0; i < solver->node_num; i++) {
		float service_edge_len = numeric_limits<float>::max();
		for (int j = 0; j < P_NUM; j++) {//找一个最近的服务点
			int centernode = solution->best_centerNodes[j];
			if (distance[i][centernode] < service_edge_len) {
				service_edge_len = distance[i][centernode];
			}
		}
		if (service_edge_len > max) {
			max = service_edge_len;
		}
	}
	if (max != solution->best_f)
		cout << "\nthe solution is error,and the correct is " << max << endl;
	else
		cout << "the solution is correct" << endl;

}

