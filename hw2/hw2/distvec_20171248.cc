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
#define LAST_INDEX(nonempty_vector) (nonempty_vector.size() - 1)
//#define PRIORITY_QUEUE


enum {
	TOP_PATH = 1,
	MSG_PATH = 2,
	CHANGES_PATH = 3
};

int add_inf(int a, int b);
bool left_is_less_inf(int a, int b);


typedef struct _entry {
	int cost;
	int next;
	std::vector<int> path;
}entry_t;


class node {
private:
	int id;
	bool is_received;
	std::vector<entry_t> routing_table; // 자기 자신의 routing table
	std::queue < std::pair<int, std::vector<entry_t>&>> msg_queue;  // 상대 노드에게 받은 메시지를 저장하는 큐
	std::vector<int> nbd_id;

public:
	node(int id, int node_num);
	int get_id(void);
	void set_table_cost(int v_id, int cost);
	void set_table_next(int where_id, int next_id);
	void append_table_path(int v_id, int path_v_id);
	void update_table(void);
	bool is_path_initialized(int dest_id);
	bool is_connected_nbd(int nbd_id);  // note that it is method for a single node
	std::vector<entry_t>& get_routing_table(void);
	void register_queue(std::vector<entry_t>& routing_table, int v_id);
	std::pair<int, std::vector<entry_t>&> msg_queue_pop(void);
	bool is_msg_queue_empty(void);
	inline void add_nbd(int nbd_id);
	void remove_nbd(int target_id);
	std::vector<int>& get_nbd_ids(void);
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
	void update_state(FILE* file);
	inline void send_table(int src_id, int dest_id);
	void communicate(void);
	void update_nodes(void);
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
	this->is_received = false;
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

void node::set_table_cost(int v_id, int cost) {
	this->routing_table[v_id].cost = cost;
}

void node::set_table_next(int where_id, int next_id) {
	this->routing_table[where_id].next = next_id;
}

void node::append_table_path(int v_id, int path_v_id) {
	this->routing_table[v_id].path.push_back(path_v_id);
}

void node::update_table(void) {
	if (this->msg_queue.size() == 0)
		return;

}

bool node::is_path_initialized(int dest_id) {
	std::vector<int>& pv = this->routing_table[dest_id].path;
	if (pv[LAST_INDEX(pv)] == dest_id)
		return true;
	return false;
}

bool node::is_connected_nbd(int nbd_id) {
	if (this->routing_table[nbd_id].cost < 0)
		return false;
	return true;
}

void node::register_queue(std::vector<entry_t>& routing_table, int v_id) {
	std::pair<int, std::vector<entry_t>&> tbl = { v_id, routing_table };
	this->msg_queue.push(tbl);
}

std::pair<int, std::vector<entry_t>&> node::msg_queue_pop(void) {
	std::pair<int, std::vector<entry_t>&> ret = this->msg_queue.front();
	this->msg_queue.pop();
	return ret;
}

std::vector<entry_t>& node::get_routing_table(void) {
	return this->routing_table;
}

bool node::is_msg_queue_empty(void) {
	return this->msg_queue.empty();
}

std::vector<int>& node::get_nbd_ids(void) {
	return this->nbd_id;
}

graph::graph(FILE* file) {
	int _num = fscanf(file, "%d", &(this->node_num));

	for (int i = 0; i < this->node_num; i++) {
		this->nodes.push_back(node(i, this->node_num));
	}

	int e_num = 0;

	for (int i = 0; i < this->node_num; i++) {
		int from = 0, to = 0, cost = 0;
		_num = fscanf(file, "%d %d %d", &from, &to, &cost);
		this->nodes[from].set_table_next(to, to);  // from 노드에서의 다음 노드를 to노드로 저장
		this->nodes[from].set_table_cost(to, cost);
		this->nodes[to].set_table_next(from, from);
		this->nodes[to].set_table_cost(from, cost);
		this->nodes[from].add_nbd(to);
		this->nodes[to].add_nbd(from);
		e_num++;
	}
	this->edge_num = e_num;
}

int graph::get_edge_num(void) {
	return this->edge_num;
}

int graph::get_node_num(void) {
	return this->node_num;
}

void graph::update_state(FILE* file) {
	int from = 0, to = 0, cost = 0;
	while (fscanf(file, "%d %d %d", &from, &to, &cost) != EOF) {
		if (cost < 0) {
			this->nodes[from].set_table_next(to, -1);
			this->nodes[from].set_table_cost(to, INT_MAX);
			this->nodes[to].set_table_next(to, -1);
			this->nodes[to].set_table_cost(from, INT_MAX);
			this->nodes[from].remove_nbd(to);
			this->nodes[to].remove_nbd(from);
		}
		else {
			this->nodes[from].set_table_cost(to, cost);
			this->nodes[to].set_table_cost(from, cost);
		}
	}
}

inline void graph::send_table(int src_id, int dest_id) {
	this->nodes[dest_id].register_queue(this->nodes[src_id].get_routing_table(), src_id);
}

void graph::communicate(void) {
	for (int i = 0; i < this->node_num; i++) {
		for (int nbd : this->nodes[i].get_nbd_ids()) {
			this->send_table(i, nbd);
		}
		this->update_nodes();
	}
}

void graph::update_nodes(void) {
	for (int i = 0; i < this->node_num; i++) {
		if (!(this->nodes[i].is_msg_queue_empty())) {
			std::vector<entry_t>& cur_table = this->nodes[i].get_routing_table();
			while (!(this->nodes[i].is_msg_queue_empty())) {
				std::pair<int, std::vector<entry_t>&> e = this->nodes[i].msg_queue_pop();
				int nbd_id = e.first;
				std::vector<entry_t>& nbd_tbl = e.second;
				for (int dest = 0; dest < this->node_num; dest++) {
					int cur_cost = cur_table[dest].cost;
				}
			}
		}
	}
}
int add_inf(int a, int b) {
	switch (a) {
		case INT_MAX:
			return INT_MAX;
		default:
			switch (b) {
				case INT_MAX:
					return INT_MAX;
				default:
					return a + b;
			}
	}
	assert(false);
}

bool left_is_less_inf(int a, int b) {
	switch (a) {
		case INT_MAX:
			return false;
		default:
			switch (b) {
				case INT_MAX:
					return true;
				default:
					return a < b;
			}
	}
	assert(false);
}