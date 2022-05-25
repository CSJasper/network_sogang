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


#define str(s) #s
#define print(x) printf("%s\n", str(x))

// use when contain method of unordered set is not supported
#define CONTAINS(hashable, element) (hashable.find(element) != hashable.end())

#define DEBUG

enum {
	TOP_PATH = 1,
	MSG_PATH = 2,
	CHANGES_PATH = 3
};

typedef struct _vertex {
	int id;
	std::vector<std::pair<int, int>> neighbor_cost;
}vertex_t;

typedef struct _entry {
	int next;
	int cost;
}entry_t;

class graph {
private:
	size_t node_num;
	size_t edge_num;

	int last_node_id;

	std::vector<int> v_list;
	std::unordered_map<int, vertex_t> v_map;
	std::vector<std::vector<entry_t>> tables;  // nodes�� id�� continuous �ϴٴ� ���� ����Ǿ��� ������ �迭�� ����ϴ� ���� �ؽ� ���� ����ϴ� �� ���� ���� �� �����ϴ�.

	~graph(void);

public:
	graph(FILE* state);

	size_t get_node_num(void);
	size_t get_edge_num(void);

	void initialize_distvec(void);

	void update_table(entry_t& source, entry_t& direct_nbd);

};


int main(int argc, char* argv[]) {
#ifndef DEBUG
	if (argc != 4) {
		print(usage: distvec topologyfile messagesfile changesfile);
	}
#endif
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

graph::graph(FILE* state) {
	int _num = fscanf(state, "%zu", &(this->edge_num));

	std::unordered_set<int> discovered;

	this->last_node_id = 0;

	for (size_t i = 0; i < edge_num; i++) {
		int v1 = 0, v2 = 0;
		int cost = 0;
		_num = fscanf(state, "%d %d %d", &v1, &v2, &cost);

		this->last_node_id = std::max({ this->last_node_id, v1, v2 });

		vertex_t v_first, v_second;
		std::pair<int, int> v_fcost = { v2, cost };
		std::pair<int, int> v_scost = { v1, cost };

		if (!CONTAINS(discovered, v1)) {
			discovered.insert(v1);
		}
		if (!CONTAINS(discovered, v2)) {
			discovered.insert(v2);
		}

		if (CONTAINS(this->v_map, v1)) {
			v_first = this->v_map[v1];
			v_first.neighbor_cost.push_back(v_fcost);
		}
		else {
			v_first.id = v1;
			v_first.neighbor_cost.push_back(v_scost); // code duplication? (please test this)
			this->v_map[v1] = v_first;
		}

		if (CONTAINS(this->v_map, v2)) {
			v_second = this->v_map[v2];
			v_second.neighbor_cost.push_back(v_scost);
		}
		else {
			v_second.id = v2;
			v_second.neighbor_cost.push_back(v_scost);
			this->v_map[v2] = v_second;
		}
	}

	this->node_num = this->v_map.size() / 2;
	assert(this->v_map.size() % 2 == 0);

	for (size_t i = 0; i < node_num; i++) {
		std::vector<entry_t> etry;
		for (size_t j = 0; j < node_num; j++) {
			entry_t e = { -1, -1 };
			etry.push_back(e);
		}
		this->tables.push_back(etry);
	}
}

graph::~graph(void) {

}

size_t graph::get_node_num(void) {
	return this->node_num;
}

size_t graph::get_edge_num(void) {
	return this->edge_num;
}

void graph::initialize_distvec(void) {
	for (int i = 0; i <= this->last_node_id; i++) {
		std::vector<std::pair<int, int>> current_nbd_cost = this->v_map[i].neighbor_cost;
		for (size_t j = 0; j < current_nbd_cost.size(); j++) {
			this->tables[i][current_nbd_cost[j].first].cost = current_nbd_cost[j].second;
			this->tables[i][current_nbd_cost[j].first].next = current_nbd_cost[j].first;
		}
	}
}

void graph::update_table(entry_t& source, entry_t& direct_nbd) {

}