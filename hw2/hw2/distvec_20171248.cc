#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cerrno>
#include <vector>
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
	std::unordered_map<unsigned, vertex_t> v_map;
	

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
	int _num = fscanf(state, "%zu", &(this->node_num));

	for (size_t i = 0; i < this->node_num; i++) {

	}

}

graph::~graph(void) {

}

size_t graph::get_node_num(void) {
	return this->node_num;
}