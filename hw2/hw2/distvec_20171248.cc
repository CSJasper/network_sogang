#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cerrno>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <climits>
#include <queue>


#define str(s) #s
#define print(x) printf("%s\n", str(x))

// use when contain method of unordered set is not supported
#define CONTAINS(hashable, element) (hashable.find(element) != hashable.end())

#define DEBUG
#define INVALID_NODE_ID -1
//#define PRIORITY_QUEUE

enum {
	TOP_PATH = 1,
	MSG_PATH = 2,
	CHANGES_PATH = 3
};

typedef struct _entry {
	int cost;
	int next;
	std::vector<int> path;
}entry_t;


class node {
private:
	int id;
	std::vector<entry_t> routing_table; // 자기 자신의 routing table
	std::queue < std::pair<int, std::vector<entry_t>>> msg_queue;  // 상대 노드에게 받은 메시지를 저장하는 큐
	std::queue<int> send_id; // 자신의 routing table이 업데이트 되었을 때, 보내야 할 라우터의 아이디를 저장 

public:
	node(int id, int node_num);
	int get_id(void);
	void receive_msg(int sender_id, std::vector<entry_t> msg);
	void set_table_cost(int v_id, int cost);
	void set_table_next(int v_id, int next_id);
	void append_table_path(int v_id, int path_v_id);
	void update_table(void);
	
};

class graph {
private:
	int edge_num;
	int node_num;
	std::vector<node> nodes;
public:
	graph(FILE* file);
	int get_edge_num(void);
	int get_node_num(void);
};

int main(int argc, char* argv[]) {
	if (argc != 4) {
		print(usage: distvec topologyfile messagesfile changesfile);
	}
	FILE* topology = fopen((const char*)argv[TOP_PATH], "r");
	FILE* msg = fopen((const char*)argv[MSG_PATH], "r");
	FILE* changes = fopen((const char*)argv[CHANGES_PATH], "r");

	if (topology == NULL || msg == NULL || changes == NULL) {
		print(Error: open input file.);
		exit(ENOENT);
	}


	/* everything is successfully done! */
	print(Complete.Output file written to output_ls.txt.);

	/* clearing */
	fclose(topology);
	fclose(msg);
	fclose(changes);
	return 0;
}

node::node(int id, int node_num) {
	this->id = id;
	for (int i = 0; i < node_num; i++) {
		entry_t e;
		e.cost = INT_MAX;
		e.next = INVALID_NODE_ID;
		this->routing_table.push_back(e);
	}
}

int node::get_id(void) {
	return this->id;
}

void node::receive_msg(int sender_id, std::vector<entry_t> msg) {
	std::pair<int, std::vector<entry_t>> p;
	p.first = sender_id;
	p.second = msg;
	this->msg_queue.push(p);
}

void node::set_table_cost(int v_id, int cost) {
	this->routing_table[v_id].cost = cost;
}

void node::set_table_next(int next_id) {
	this->routing_table
}

void node::append_table_path(int v_id, int path_v_id) {
	this->routing_table[v_id].path.push_back(path_v_id);
}

void node::update_table(void) {
	if (this->msg_queue.size() == 0)
		return;

}

graph::graph(FILE* file) {
	int _num = fscanf(file, "%d", &(this->node_num));

	for (int i = 0; i < this->node_num; i++) {
		this->nodes.push_back(node(i, this->node_num));
	}

	for (int i = 0; i < this->node_num; i++) {
		int from = 0, to = 0, cost = 0;
		_num = fscanf(file, "%d %d %d", &from, &to, &cost);
		this->nodes[from].set_table_next()

	}
}