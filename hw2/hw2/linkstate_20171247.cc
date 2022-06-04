#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <climits>


#define str(s) #s
#define print(x) printf("%s\n", str(x))

#define INF INT_MAX
#define NO_PREV -1



typedef struct _entry {
	int next;
	int cost;
	int prev;
}entry_t;

class node {
private:
	int id;
	int total_node_num;
	std::vector<int> nbd_ids;
	std::unordered_map<int, int> nbd_id_cost;
	std::vector<entry_t> routing_table;
public:
	node(int id, int total_node_num);
	~node(void);
	int get_id(void);
	int get_total_node_num(void);
	std::vector<int>& get_nbds(void);
	std::vector<entry_t>& get_routing_table(void);

	void add_nbd(const int id, const int cost);
	void remove_nbd(const int id);
	void set_cost(const int target_id, const int cost);
	void set_next(const int targrt_id, const int next_id);
	void set_prev(const int target_id, const int prev_id);



};

struct p_cmp {
	bool operator()(std::pair<node&, int> a, std::pair<node&, int> b) {
		if (a.second != b.second) {
			return a.second < b.second;
		}
		return a.first.get_id() < b.first.get_id();  // tie breaking rule 2
	}
};

class graph {
private:
	int edge_num;
	int node_num;
	std::vector<node> nodes;
	std::priority_queue<std::pair<node&, int>, std::vector<std::pair<node&, int>>, p_cmp> open;
public:
	graph(FILE* file);
	~graph(void);
	int get_edge_num(void);
	int get_node_num(void);
	node& get_node(const int id);
	void run(const int start_id);

	void modify_nbd_cost(const int id1, const int id2, const int cost);
};

int add_inf(int a, int b);
bool left_is_less_inf(int left, int right);


int main(int argc, char* argv[]) {

	return 0;
}

node::node(int id, int total_node_num) {
	this->id = id;
	this->total_node_num = total_node_num;
	// initialize routing table
	for (int i = 0; i < total_node_num; i++) {
		this->routing_table[i].cost = INF;
		this->routing_table[i].next = -1;
		this->routing_table[i].prev = NO_PREV;
	}
	this->routing_table[id].cost = 0;
	this->routing_table[id].next = id;
	this->routing_table[id].prev = id;
}

int add_inf(int a, int b) {
	switch (a) {
		case INF:
			return INF;
		default:
			switch (b) {
				case INF:
					return INF;
				default:
					return a + b;
			}
	}
	// intentional breakthrough
	assert(false);
}

bool left_is_less_inf(int left, int right) {
	switch (left) {
		case INF:
			return false;
		default:
			switch (right) {
				case INF:
					return true;
				default:
					return left < right;
			}
	}
	// intentional breakthrough
	assert(false);
}

void graph::modify_nbd_cost(const int id1, const int id2, const int cost) {
	node& n1 = this->get_node(id1);
	node& n2 = this->get_node(id2);

	switch (cost) {
		case INF:
			n1.remove_nbd(id2);
			n2.remove_nbd(id1);
			// intentional breakthrough
		default:
			

	}
}

void node::add_nbd(const int id, const int cost) {
	this->nbd_ids.push_back(id);
	this->nbd_id_cost[id] = cost;
	if (this->nbd_ids.size() != this->nbd_id_cost.size()) {
		for (size_t i = 0; i < this->nbd_ids.size(); i++) {
			if (this->nbd_ids[i] != id) {
				this->nbd_ids.erase(this->nbd_ids.begin() + i);
				break;
			}
		}
	}
	assert(this->nbd_ids.size() == this->nbd_id_cost.size(), "The size of list and hash map of same ids are not synchronized.");
}

void node::remove_nbd(const int id) {
	this->nbd_id_cost.erase(id);
	for (size_t i = 0; i < this->nbd_ids.size(); i++) {
		if (this->nbd_ids[i] == id) {
			this->nbd_ids.erase(this->nbd_ids.begin() + i);
			break;
		}
	}
}