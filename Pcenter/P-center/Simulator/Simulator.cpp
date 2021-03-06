// Simulator.cpp: 定义控制台应用程序的入口点。
//

#include"stdafx.h"
#include"Pcenter.h"
using namespace std;

int P_NUM;
float TRUE_BEST_F;
bool increase_compare(const DisNode &a, const DisNode &b) {
	return (a.distance < b.distance);
}
int main()
{
	string instanceList = INSTANCE_LIST;
	ifstream readFile(instanceList);
	string str;
	int node_num;
	while (getline(readFile, str) &&str !="EOF") {
		string instanceName;
		if (str.find("//") != string::npos) continue;
		stringstream ss(str);
		ss >> instanceName >>node_num>> P_NUM >> TRUE_BEST_F;
		if (instanceName.find("pmed") != string::npos) {
			instanceName = instanceName + ".txt";
		}
		else {
			instanceName = instanceName + ".tsp";
		}
		testInstance(instanceName, node_num,P_NUM, TRUE_BEST_F);
	}
	return 0;
}
void testInstance(string name,int node_num,int p_num,int true_best_f) {
	string instancePath = INSTANCE_DIR + name;
	PcenterSolver *solver = new PcenterSolver(instancePath,node_num,p_num,true_best_f);
	solver->randseed = (int)time(NULL);
	cout << SEPARATOR << "Solve "<<name<<" The init seed is " << solver->randseed << SEPARATOR << endl;
	srand(solver->randseed);
	//srand(1539183026);
	PcenterSolver_solve(solver);
}

int PcenterSolver_solve(PcenterSolver *solver) {
	clock_t begin = clock();
	init_solution(solver);
	clock_t init_time = clock();
	
	int iter = 1;
	int best_f = solver->solution->best_f, cnt = 0;
	while (iter<MAX_ITERATIONS) {
		Move *move = find_move(solver, iter);
		if (move == NULL) {
			cout << "Move is NULL" << endl;
			break;
		}
		make_move(move, solver, iter);
		if (abs(solver->solution->best_f - TRUE_BEST_F)<0.01) {
			break;
		}
		/*if (best_f < solver->solution->best_f) {
			cnt = 0;
		}
		else
			cnt++;
		if (cnt > MAX_NO_IMPROVE_ITERATIONS)
			break;*/
		if ((clock() - init_time)*1.0 / CLOCKS_PER_SEC > 10)break;
		iter++;
	}
	clock_t end_time = clock();
	
	solver->solution->print_solution(1);
	check_end_solution(solver);
	cout << endl<<SEPARATOR << SEPARATOR << endl;
	cout << "P_NUM: " << P_NUM << "  true_best: " << TRUE_BEST_F << endl;
	cout << "init time : " << (init_time - begin)*1.0 / CLOCKS_PER_SEC << "s" << endl;
	cout << "optimize time : " << (end_time - init_time)*1.0 / CLOCKS_PER_SEC << "s" << endl;
	cout << "total time : " << (end_time - begin)*1.0 / CLOCKS_PER_SEC << "s" << endl;
	cout << SEPARATOR << SEPARATOR << endl;

	return 1;
}
void copy_F_D(int(*dst_F)[2], float(*dst_D)[2], int(*src_F)[2], float(*src_D)[2], int node_num) {
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
	int node_num = solver->node_num, max_num, max_node;
	int(*F)[2] = solver->F;
	float(*D)[2] = solver->D, max = -1, temp_max = -1;
	for (int i = 0; i < node_num; i++) {
		/*float service_edge_len = numeric_limits<float>::max();

		for (int j = 0; j < P_NUM; j++) {//找一个最近的服务点
		int centernode = solution->centerNodes[j];
		if (distance[i][centernode] < service_edge_len) {
		service_edge_len = distance[i][centernode];
		}
		}
		if (distance[o_node][i] < service_edge_len)
		service_edge_len = distance[o_node][i];
		if (service_edge_len>temp_max)
		temp_max = service_edge_len;*/
		if (distance[o_node][i] < D[i][0]) {//o_node成为i的服务站点
			F[i][1] = F[i][0];
			D[i][1] = D[i][0];
			F[i][0] = o_node;
			D[i][0] = distance[o_node][i];
		}
		else if (distance[o_node][i] < D[i][1]) {//o_node成为i的老二
			F[i][1] = o_node;
			D[i][1] = distance[o_node][i];
		}
		if (D[i][0] > max) {//D[i][0]为i到其服务点的距离,F[i][0]为i的服务点
			max_num = 1;
			max = D[i][0];
			max_node = i;
		}
		else if (D[i][0] == max) {
			max_num++;
			if (rand() % max_num < 1) {//随机从最长服务边中选一条
				max_node = i;
			}
		}
	}

	solution->new_f = max;
	solution->longest_edge_node = max_node;
}
void deleteCenterNodeAndUpdateF_D(PcenterSolver *solver, int node) {
	Solution *solution = solver->solution;
	int *centerNodes = solution->centerNodes;
	float **distance = solver->distance;
	int node_num = solver->node_num, max_num, min_node, max_node;
	int(*F)[2] = solver->F;
	float(*D)[2] = solver->D, min, max = -1;
	for (int i = 0; i < node_num; i++) {
		if (F[i][0] == node) {
			F[i][0] = F[i][1];
			D[i][0] = D[i][1];
			min = FLOAT_MAX;
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
		}
		else if (F[i][1] == node) {
			min = FLOAT_MAX;
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

		}
		if (D[i][0] > max) {//D[i][0]为i到其服务点的距离,F[i][0]为i的服务点
			max_num = 1;
			max = D[i][0];
			max_node = i;
		}
		else if (D[i][0] == max) {
			max_num++;
			if (rand() % max_num < 1) {//随机从最长服务边中选一条
				max_node = i;
			}
		}
	}
	if (solution->f != max) {
		cout << "deleteCenterNodeAndUpdateF_D error!!" << endl;
		cout << "the solution is " << solution->f << " and the dmax is" << max << endl;
		check_D(solver);
	}
	solution->longest_edge_node = max_node;
}
Move *find_move(PcenterSolver *solver, int iter) {
	Move *move, *best_move, *tabu_move, *best_tabu_move;
	Solution *solution = solver->solution;
	int *indexInCenterNodeArray = solution->indexInCenterNodeArray;
	int *centerNodes = solution->centerNodes;
	int(*F)[2] = solver->F, (*recovery_F)[2];
	float(*D)[2] = solver->D, (*recovery_D)[2];
	int node_num = solver->node_num, longest_edge_node = solution->longest_edge_node;
	float **distance = solver->distance, f = solution->f;
	int all_tabu = 1;
	int *tabu_o2c = solver->tabu_o2c,*tabu_c2o = solver->tabu_c2o;
	int bft_node[K], bft_num = 0;//备选的普通节点
	DisNode *disnode = new DisNode[node_num];
	for (int i = 0; i < node_num ; i++) {//TODO:这里需要考虑一下怎么选K个
		if (indexInCenterNodeArray[i] == -1 && distance[longest_edge_node][i] < f) {//从非服务站点中寻找
			disnode[bft_num].node = i;
			disnode[bft_num++].distance = distance[longest_edge_node][i];
		}
	}
	if(K < bft_num)
		partial_sort(disnode, disnode + K, disnode + bft_num, increase_compare);
	if (bft_num == 0) return NULL;
	move = new Move, best_move = new Move, tabu_move = new Move, best_tabu_move = new Move;
	float best = FLOAT_MAX, tabu_best = FLOAT_MAX;
	recovery_F = new int[node_num][2];
	recovery_D = new float[node_num][2];

	bft_num = min(K, bft_num);
	for (int i = 0; i < bft_num; i++) {
		int o_node = disnode[i].node;

		copy_F_D(recovery_F, recovery_D, F, D, node_num);
		addCenterNodeAndUpdateF_D(solver, o_node);//添加o_node
		float min = FLOAT_MAX, tabu_min = FLOAT_MAX;
		for (int j = 0; j < P_NUM; j++) {//TODO:将这里的时间复杂度降低一些
			int center_node = centerNodes[j];//假如删除掉center_node最长服务边会变成什么
			float max = -1;
			for (int k = 0; k < node_num; k++) {
				if (F[k][0] == center_node) {
					if (D[k][1] > max) {//max为原来服务点为center_node的节点删除掉center_node之后的最长服务边
						max = D[k][1];
					}
				}
			}
			if (tabu_c2o[center_node]>iter && tabu_o2c[o_node] >iter) {//说明仍然处于禁忌中
				if (max < tabu_min) {
					tabu_min = max;
					tabu_move->o_node = o_node;
					tabu_move->center_node = center_node;
				}
			}
			else {
				if (max < min) {
					all_tabu = 0;
					min = max;
					move->o_node = o_node;//TODO:这个可以删掉
					move->center_node = center_node;
				}//TODO:相等时随机选择一个
			}

		}
		min = max(solution->new_f, min);
		tabu_min = max(solution->new_f, tabu_min);
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
		copy_F_D(F, D, recovery_F, recovery_D, node_num);
	}
	delete[]disnode;
	delete[]recovery_D;
	delete[]recovery_F;
	delete move;
	if ((tabu_best < best &&tabu_best < solution->best_f)||all_tabu) {//解禁策略
		cout << "aspiration" << endl;
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
void make_move(Move *move, PcenterSolver *solver, int iter) {
	Solution *solution = solver->solution;
	int node1, node2;
	solution->addCenterNode(move->o_node);
	solution->deleteCenterNode(move->center_node);
	if (solution->f < solution->best_f) {
		solution->best_f = solution->f;
		solution->copyCenterNodesTobest();
	}
	solver->tabu_c2o[move->o_node] = iter + P_NUM / 10 + rand() % 10;
	solver->tabu_o2c[move->center_node] = iter + (solver->node_num-P_NUM )/ 10 + rand() % 100;

	addCenterNodeAndUpdateF_D(solver, move->o_node);
	deleteCenterNodeAndUpdateF_D(solver, move->center_node);

#ifdef LOG_MOVE
	cout << SEPARATOR << "Iteration " << iter << SEPARATOR << endl;
	//cout << "The move is " << move->o_node << " and " << move->center_node << endl;
	//cout << "The new solution is " << solution->f << endl;
	check_current_solution(solver);
	check_D(solver);
	cout << SEPARATOR << SEPARATOR << endl;
#endif // LOG_MOVE


}
bool decrease_compare(const DisNode &a, const DisNode &b) {
	return (a.distance > b.distance);
}

void init_solution(PcenterSolver *solver) {
	int node_num = solver->node_num;
	Solution *solution = solver->solution;
	NodesOfCN *nodesOfCenter = solver->nodesOfCenter;
	vector <DisNode> disNode;
	float **distance = solver->distance, longest_len = -1;
	int node = rand() % node_num, longest_node;//首先随机选一个node作为Center
	disNode.resize(node_num);
	while (solution->centerNum < P_NUM) {//选择某些node为Center
		solution->addCenterNode(node);
		for (int i = 0; i < node_num; i++) {
			float service_edge = FLOAT_MAX;
			for (int j = 0; j < solution->centerNum; j++) {
				int center = solution->centerNodes[j];
				if (distance[i][center] < service_edge) {
					service_edge = distance[i][center];
				}
			}
			if (service_edge > longest_len) {
				longest_len = service_edge;
				longest_node = i;
			}
		}

		for (int i = 0; i < node_num; i++) {
			if (solution->indexInCenterNodeArray[i] != -1)//已经被选为中心的不再选择
				disNode[i].distance = FLOAT_MAX;
			else
				disNode[i].distance = distance[longest_node][i];
			disNode[i].node = i;
		}
		partial_sort(disNode.begin(), disNode.begin() + K, disNode.end(), increase_compare);
		node = disNode[rand() % K].node;//从前K个距离最近的里面随机挑一个node
	}
	solution->copyCenterNodesTobest();
	int *centerNodes = solution->centerNodes, max_num, max_node;
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
			max_num++;
			if (rand() % max_num < 1) {//随机从最长服务边中选一条
				max_node = i;
			}
		}
	}
	solution->f = max;
	solution->best_f = max;
	solution->longest_edge_node = max_node;
#ifdef LOG_ON
	cout << "The init solution is " << solution->f << endl;
#endif // LOG_ON
	check_current_solution(solver);
	check_D(solver);
}

void check_end_solution(PcenterSolver *solver) {
	Solution *solution = solver->solution;
	float **distance = solver->distance;
	if (solution->centerNum != P_NUM) {
		cout << "the center node num is error ";
	}
	float max = -1;
	for (int i = 0; i < solver->node_num; i++) {
		float service_edge_len = FLOAT_MAX;
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
		cout << "\nthe current solution is error,and the correct is " << max << endl;
	else
		cout << "the current solution is correct,and the correct is " << max << endl;

}
void check_current_solution(PcenterSolver *solver) {
	Solution *solution = solver->solution;
	float **distance = solver->distance;
	if (solution->centerNum != P_NUM) {
		cout << "the center node num is error ";
	}
	float max = -1;
	for (int i = 0; i < solution->centerNum; i++) {
		int center1 = solution->centerNodes[i];
		for (int j = 0; j < solution->centerNum; j++) {
			if (i != j && center1 == solution->centerNodes[j]) {
				cout << "The center node overlap" << endl;
				exit(1);
			}
		}
	}
	for (int i = 0; i < solver->node_num; i++) {
		float service_edge_len = FLOAT_MAX;
		for (int j = 0; j < solution->centerNum; j++) {//找一个最近的服务点
			int centernode = solution->centerNodes[j];
			if (distance[i][centernode] < service_edge_len) {
				service_edge_len = distance[i][centernode];
			}
		}
		if (service_edge_len > max) {
			max = service_edge_len;
		}
	}

	if (max != solution->f)
		cout << "\nthe current solution is error,and the correct is " << max << endl;
	else
		cout << "the current solution is correct,and the correct is " << max << endl;

}

void check_D(PcenterSolver *solver) {
	float D_max = -1;
	for (int i = 0; i < solver->node_num; i++) {
		if (solver->D[i][0] > D_max)
			D_max = solver->D[i][0];
	}
	if (D_max != solver->solution->f) {
		cout << "ERROR !  The D_max is " << D_max << " and the solution is " << solver->solution->f << endl;
	}
	else
		cout << "the D list is correct and The D_max is " << D_max << endl;
}
