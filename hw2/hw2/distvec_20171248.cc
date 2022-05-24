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

enum {
	TOP_PATH = 1,
	MSG_PATH = 2,
	CHANGES_PATH = 3
};

typedef struct _vertex {
	unsigned id;
	std::vector<unsigned> neighbor;
}vertex_t;

typedef struct _edge {
	unsigned ordered_vertices[2];
	int cost;
}edge_t;

class graph {
private:
	size_t node_num;
	size_t edge_num;
	std::unordered_map<unsigned, vertex_t> v_map;
	std::unordered_map<unsigned*, edge_t> e_map;
	~graph(void);

public:
	graph(FILE* state);

	size_t get_node_num(void);


};

class distvec {
private:

public:
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

graph::graph(FILE* state) {
	int _num = fscanf(state, "%zu", &(this->edge_num));

	for (size_t i = 0; i < this->edge_num; i++) {
		unsigned from = 0, to = 0;
		int cost = 0;
		_num = fscanf(state, "%u %u %d", &from, &to, &cost);
		unsigned e[2] = { std::min(from, to), std::max(from, to) };
		// 이 방법은 같은 edge가 들어왔을 때 unordered_map의 상태를 정의하지 않습니다. 그러므로 input에서 똑같은 입력을 하지 않는다고 가정합니다. (이 부분에 대한 error handling은 아직 구현되지 않았습니다.)
		edge_t edge;
		edge.ordered_vertices[0] = e[0];
		edge.ordered_vertices[1] = e[1];
		edge.cost = cost;
		this->e_map[e] = edge;
	}

	for (std::unordered_map<unsigned*, edge_t>::iterator iter = this->e_map.begin(); iter != e_map.end(); iter++) {
		unsigned* p = iter->first;

	}

}

graph::~graph(void) {

}

size_t graph::get_node_num(void) {
	return this->node_num;
}