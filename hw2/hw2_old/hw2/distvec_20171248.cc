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
#include <set>
#include <map>
#include <climits>
#include <queue>
#include <string>
#include <sstream>
#include <utility>

namespace patch {
	template <typename T> 
	std::string to_string(const T& n) {
		std::ostringstream stm;
		stm << n;
		return stm.str();
	}
}


#define str(s) #s
#define print(x) printf("%s\n", str(x))

// use when contain method of unordered set is not supported
#define CONTAINS(hashable, element) (hashable.find(element) != hashable.end())

#define DEBUG
#define INVALID_NODE_ID -1
#define LAST_INDEX(nonempty_vector) (nonempty_vector.size() - 1)
//#define PRIORITY_QUEUE

int debug = 0;

enum {
	TOP_PATH = 1,
	MSG_PATH = 2,
	CHANGES_PATH = 3,
	MAX_MSG_SIZE = 1010,
	MAX_LINE = 100
};

int add_inf(int a, int b);
bool left_is_less_inf(int a, int b);
std::vector<int> one_by_all_concatenate(std::vector<int>& one_v, std::vector<int>& v2);
template<typename T>
inline T* ref_to_pointer(T& a);

template<typename T>
inline T& pointer_to_ref(T* a);

typedef struct _entry {
	int cost;
	int next;
	std::vector<int> path;
}entry_t;


class node {
private:
	int id;
	bool is_changed;
	std::vector<entry_t> routing_table; // 자기 자신의 routing table
	std::queue < std::pair<int, std::vector<entry_t>*> > msg_queue;  // 상대 노드에게 받은 메시지를 저장하는 큐
	std::map<int, int> nbd_ids_cost;
	std::vector<int> nbd_ids;

public:
	node(int id, int node_num);
	int get_id(void);
	void set_table_cost(int v_id, int cost);
	void set_table_next(int where_id, int next_id);
	void append_table_path(int v_id, int path_v_id);
	bool is_path_initialized(int dest_id);
	bool is_connected_nbd(int nbd_id);  // note that it is method for a single node
	std::vector<entry_t>& get_routing_table(void);
	void register_queue(std::vector<entry_t>& routing_table, int v_id);
	std::pair<int, std::vector<entry_t>*> msg_queue_pop(void);
	bool is_msg_queue_empty(void);
	inline void add_nbd_cost(int nbd_id, int cost);  // 처음 초기화 시에 이 노드의 이웃이 어떤 노드인지 저장하려는 메서드
	inline void remove_nbd(int target_id);  // 나중에 이 노드의 이웃이 바뀐다면 그 노드를 이웃리스트에서 제거하는 메서드
	std::vector<int>& get_nbd_ids(void);
	int get_direct_cost(int nbd_id);
	bool get_is_changed(void);
	void set_is_changed(bool to_what);
	void print_routing_table(FILE* file);
	bool is_direct_nbd(int target_id);
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
	void update_state(int, int, int);
	inline void send_table(int src_id, int dest_id);
	void communicate(void);
	void update_nodes(void);
	inline void set_path_vector(int src_id, int to_id, std::vector<int>& path);
	void recompute_cost(int target_id);
	void print_message(int src_id, int dest_id, const char* msg, FILE* file);
	void print_tables(FILE* file);
};

int main(int argc, char* argv[]) {
	if (argc != 4) {
		print(usage: distvec topologyfile messagesfile changesfile);
	}
	FILE* topology = fopen((const char*)argv[TOP_PATH], "r");
	FILE* msg = fopen((const char*)argv[MSG_PATH], "r");
	FILE* changes = fopen((const char*)argv[CHANGES_PATH], "r");
	FILE* output = fopen("output_dv.txt", "w");

	if (topology == NULL || msg == NULL || changes == NULL) {
		print(Error: open input file.);
		exit(ENOENT);
	}

	char* raw_msg = (char*)malloc(sizeof(char) * MAX_MSG_SIZE);
	char* a_msg = (char*)malloc(sizeof(char) * MAX_MSG_SIZE);
	std::vector<std::string> msgs;
	std::vector<std::pair<int, int> > src_dst_msg;
	while (true) {
		memset((void*)raw_msg, 0, MAX_MSG_SIZE);
		fgets(raw_msg, MAX_MSG_SIZE, msg);
		int src_id = 0, dst_id = 0;
		int _num = sscanf(raw_msg, "%d %d %[^\n]", &src_id, &dst_id, a_msg);
		if (_num < 3 || _num == EOF) {
			break;
		}
		msgs.push_back(std::string(a_msg));
		std::pair<int, int> el(src_id, dst_id);
		src_dst_msg.push_back(el);
	}

	free(raw_msg);
	raw_msg = NULL;
	free(a_msg);
	a_msg = NULL;

	graph g(topology);

	g.communicate();
	g.print_tables(output);

	for (size_t i = 0; i < msgs.size(); i++) {
		g.print_message(src_dst_msg[i].first, src_dst_msg[i].second, msgs[i].c_str(), output);
	}
	if(msgs.size() != 0)
		fprintf(output, "\n");

	char changes_txt[50] = { 0, };

	while (true) {
		memset(changes_txt, 0, 50);
		fgets(changes_txt, 50, changes);
		int src = 0, dest = 0, c_cost = 0;
		int _num = sscanf(changes_txt, "%d %d %d", &src, &dest, &c_cost);
		if (_num == 0 || _num == EOF) {
			break;
		}
		g.update_state(src, dest, c_cost);
		g.communicate();
		g.print_tables(output);
		for (size_t i = 0; i < msgs.size(); i++) {
			g.print_message(src_dst_msg[i].first, src_dst_msg[i].second, msgs[i].c_str(), output);
		}
		fprintf(output, "\n");
		debug++;
	}

	/* everything is successfully done! */
	print(Complete.Output file written to output_dv.txt.);

	/* clearing */
	fclose(topology);
	fclose(msg);
	fclose(changes);
	fclose(output);
	return 0;
}

node::node(int id, int node_num) {
	this->id = id;
	this->is_changed = false;
	for (int i = 0; i < node_num; i++) {
		entry_t e;
		e.cost = INT_MAX;
		e.next = INVALID_NODE_ID;
		e.path.push_back(this->id);
		this->routing_table.push_back(e);
	}
	this->routing_table[this->id].cost = 0;
	this->routing_table[this->id].next = this->id;
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
	std::pair<int, std::vector<entry_t>*> tbl(v_id, &routing_table);
	this->msg_queue.push(tbl);
}

std::pair<int, std::vector<entry_t>*> node::msg_queue_pop(void) {
	std::pair<int, std::vector<entry_t>*> ret = this->msg_queue.front();
	this->msg_queue.pop();
	return ret;
}

std::vector<entry_t>& node::get_routing_table(void) {
	return this->routing_table;
}

bool node::is_msg_queue_empty(void) {
	return this->msg_queue.empty();
}

bool node::get_is_changed(void) {
	return this->is_changed;
}

void node::set_is_changed(bool to_what) {
	this->is_changed = to_what;
}

graph::graph(FILE* file) {
	fscanf(file, "%d", &(this->node_num));

	for (int i = 0; i < this->node_num; i++) {
		this->nodes.push_back(node(i, this->node_num));
	}

	int e_num = 0;
    int from = 0, to = 0, cost = 0;

	while(fscanf(file, "%d %d %d", &from, &to, &cost) != EOF) {
		this->nodes[from].set_table_next(to, to);  // from 노드에서의 다음 노드를 to노드로 저장
		this->nodes[from].set_table_cost(to, cost);
		this->nodes[to].set_table_next(from, from);
		this->nodes[to].set_table_cost(from, cost);
		this->nodes[from].add_nbd_cost(to, cost);
		this->nodes[to].add_nbd_cost(from, cost);
		this->nodes[from].set_is_changed(true);
		this->nodes[to].set_is_changed(true);
		this->nodes[from].append_table_path(to, to);
		this->nodes[to].append_table_path(from, from);
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

void graph::update_state(int src_id, int dest_id, int cost) {
	if (cost < 0) {
		this->nodes[src_id].set_table_next(dest_id, -1);
		this->nodes[src_id].set_table_cost(dest_id, INT_MAX);
		this->nodes[dest_id].set_table_next(src_id, -1);
		this->nodes[dest_id].set_table_cost(src_id, INT_MAX);
		this->nodes[src_id].remove_nbd(dest_id);
		this->nodes[dest_id].remove_nbd(src_id);
		std::vector<int> sd_path(1, src_id);
		std::vector<int> ds_path(1, dest_id);
		this->set_path_vector(src_id, dest_id, sd_path);
		this->set_path_vector(dest_id, src_id, ds_path);
		this->nodes[src_id].remove_nbd(dest_id);
		this->nodes[dest_id].remove_nbd(src_id);
	}
	else {
		this->nodes[src_id].set_table_cost(dest_id, cost);
		this->nodes[dest_id].set_table_cost(src_id, cost);
		this->nodes[src_id].add_nbd_cost(dest_id, cost);
		this->nodes[dest_id].add_nbd_cost(src_id, cost);
	}

	this->nodes[src_id].set_is_changed(true);
	this->nodes[dest_id].set_is_changed(true);
}

inline void graph::send_table(int src_id, int dest_id) {
	this->nodes[dest_id].register_queue(this->nodes[src_id].get_routing_table(), src_id);
}

void graph::communicate(void) {
	bool exists_change = false;
	while (true) {
		for (int i = 0; i < this->node_num; i++) {
			if (this->nodes[i].get_is_changed()) {
				std::vector<int>& nbds = this->nodes[i].get_nbd_ids();
				for (size_t n_idx = 0; n_idx < nbds.size(); n_idx++) {
					this->send_table(i, nbds[n_idx]);
					this->recompute_cost(nbds[n_idx]);
				}
				this->nodes[i].set_is_changed(false);
				exists_change = true;
			}
		}
		this->update_nodes();

		if (!exists_change) {
			break;
		}

		exists_change = false;
	}
}

void graph::update_nodes(void) {
	for (int i = 0; i < this->node_num; i++) {
		while (!this->nodes[i].is_msg_queue_empty()) {
			std::vector<entry_t>& cur_table = this->nodes[i].get_routing_table();
			std::pair<int, std::vector<entry_t>*> e = this->nodes[i].msg_queue_pop();
			int nbd_id = e.first;
			std::vector<entry_t>& nbd_tbl = pointer_to_ref(e.second);
			for (int dest = 0; dest < this->node_num; dest++) {

				if (dest == i)
					continue;

				int cur_cost = cur_table[dest].cost;
				int cur_nbd_cost = this->nodes[i].get_direct_cost(nbd_id);
				int from_nbd_dest_cost = nbd_tbl[dest].cost;
				if (left_is_less_inf(add_inf(cur_nbd_cost, from_nbd_dest_cost), cur_cost)) {
					cur_table[dest].cost = add_inf(cur_nbd_cost, from_nbd_dest_cost);
					cur_table[dest].next = nbd_id;
					cur_table[dest].path = one_by_all_concatenate(cur_table[dest].path, nbd_tbl[dest].path);
					this->nodes[i].set_is_changed(true);
				}
				else if (add_inf(cur_nbd_cost, from_nbd_dest_cost) == cur_cost) {
					if (nbd_id == dest)
						continue;
					if (nbd_id < cur_table[dest].next) {
						cur_table[dest].next = nbd_id;
						cur_table[dest].path = one_by_all_concatenate(cur_table[dest].path, nbd_tbl[dest].path);
						this->nodes[i].set_is_changed(true);
					}
				}
			}
		}
	}
}

inline void graph::set_path_vector(int src_id, int dest_id, std::vector<int>& path) {
	this->nodes[src_id].get_routing_table()[dest_id].path = path;
}

void graph::recompute_cost(int target_id) {
	bool exist_changes = false;
	std::vector<entry_t>& cur_table = this->nodes[target_id].get_routing_table();
	for (int i = 0; i < this->node_num; i++) {
		if (i == target_id)
			continue;
		std::vector<int>& path = cur_table[i].path;
		if (path[path.size() - 1] != i)
			continue;
		int cost = 0;
		for (size_t j = 0; j < path.size() - 1; j++) {
			if (this->nodes[path[j]].is_direct_nbd(path[j + 1])) {
				cost = add_inf(cost, this->nodes[path[j]].get_direct_cost(path[j + 1]));
			}
			else {
				cost = INT_MAX;
				break;
			}
		}
		if (cost != cur_table[i].cost) {
			exist_changes = true;
			cur_table[i].cost = cost;
		}
	}
	if (exist_changes) {
		this->nodes[target_id].set_is_changed(true);
	}
}

void graph::print_message(int src_id, int dest_id, const char* msg, FILE* file) {
	std::string paths("");
	std::vector<int>& p_v = this->nodes[src_id].get_routing_table()[dest_id].path;
	if (p_v[0] != src_id || p_v[p_v.size() - 1] != dest_id) {
		fprintf(file, "from %d to %d cost infinite hops unreachable message %s\n", src_id, dest_id, msg);

		return;
	}

	for (size_t i = 0; i < p_v.size() - 1; i++) {
		paths += patch::to_string(p_v[i]) + " ";
	}
	int cost = this->nodes[src_id].get_routing_table()[dest_id].cost;
	fprintf(file, "from %d to %d cost %d hops %s message %s\n", src_id, dest_id, cost, paths.c_str(), msg);
}

void graph::print_tables(FILE* file) {
	for (int i = 0; i < this->node_num; i++) {
		this->nodes[i].print_routing_table(file);
	}
}

inline void node::add_nbd_cost(int nbd_id, int cost) {
	this->nbd_ids_cost[nbd_id] = cost;
	this->nbd_ids.push_back(nbd_id);
	if (this->nbd_ids.size() != this->nbd_ids_cost.size()) {
		for (size_t i = 0; i < this->nbd_ids.size(); i++) {
			if (this->nbd_ids[i] == nbd_id) {
				this->nbd_ids.erase(this->nbd_ids.begin() + i);
				break;
			}
		}
	}
	assert(this->nbd_ids.size() == this->nbd_ids_cost.size());
}

inline void node::remove_nbd(int target_id) {
	this->nbd_ids_cost.erase(target_id);
	for (size_t i = 0; i < this->nbd_ids.size(); i++) {
		if (this->nbd_ids[i] == target_id) {
			this->nbd_ids.erase(this->nbd_ids.begin() + i);
			break;
		}
	}
}

void node::print_routing_table(FILE* file) {
	for (size_t dest = 0; dest < this->routing_table.size(); dest++) {
		fprintf(file, "%zu %d %d\n", dest, this->routing_table[dest].next, this->routing_table[dest].cost);
	}
	fprintf(file, "\n");
}

std::vector<int>& node::get_nbd_ids(void) {
	return this->nbd_ids;
}

int node::get_direct_cost(int nbd_id) {
	return this->nbd_ids_cost[nbd_id];
}

bool node::is_direct_nbd(int target_id) {
	std::vector<int>& nbds = this->nbd_ids;
	for (size_t i = 0; i < nbds.size(); i++) {
		int id = nbds[i];
		if (id == target_id)
			return true;
	}
	return false;
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

std::vector<int> one_by_all_concatenate(std::vector<int>& one_v, std::vector<int>& v2) {
	std::vector<int> ret;
	ret.push_back(one_v[0]);
	for (size_t i = 0; i < v2.size(); i++) {
		ret.push_back(v2[i]);
	}
	return ret;
}

template<typename T>
inline T* ref_to_pointer(T& a) {
	return &a;
}
template<typename T>
inline T& pointer_to_ref(T* a) {
	return *a;
}