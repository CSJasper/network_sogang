#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <map>
#include <set>
#include <queue>
#include <vector>
#include <climits>
#include <cerrno>
#include <stack>
#include<string>


#define str(s) #s
#define print(x) printf("%s\n", str(x))

#define INF INT_MAX
#define NO_PREV -1

enum {
    TOP_PATH = 1,
    MSG_PATH = 2,
    CHANGES_PATH = 3
};

typedef struct _entry{
    int cost;
    int next;
}entry_t;

template<typename T>
T min(T a, T b);

template<typename T>
T max(T a, T b);

int add_inf(const int a, const int b);

bool left_is_less_inf(const int a, const int b);

typedef struct _lsp {
    int sender_id;
    std::vector<std::pair<int, int> > id_dist;
    int sequence_num;
} LSP_t;

struct q_cmp {
    bool operator()(const std::pair<int, int> a, const std::pair<int, int> b) {
        if (a.first == b.first)
            return a.second < b.second;
        else
            return a.first < b.first;
    }
};

class database {
private:
    int router_num;
    int edge_num;
    std::vector<std::vector<int> > edge_cost;
    std::vector<int> seq_nums;
    std::map<int, std::vector<int> > nbds;
    std::queue<int> dijkstra_queue;

public:
    database(int rounter_num);

    std::vector<int> &get_nbds(const int id);

    int get_edge_cost(const int id1, const int id2);

    int get_router_num(void);

    int get_edge_num(void);

    void set_edge_cost(const int id1, const int id2, const int cost);

    void set_edge_num(const int edge_num);

    void add_nbd(const int to_what, const int nbd_id);

    void remove_nbd(const int from_what, const int target_id);

    int get_seq_num(int target_id);

    void increase_seq_num(int target_id);

    void wait_dijkstra(int target_id);

    int pop_dijkstra_queue(void);

    bool is_nbd(const int src, const int target);

    bool is_queue_empty(void);


};

class node {
private:
    static database* db;
    int id;
    int total_node_num;
    bool exists_change;
    std::vector<entry_t> routing_table;

    std::vector<int> dist;
    std::vector<int> prev;

    std::queue<LSP_t> packet_received;


public:
    node(const int id, const int total_node_num);

    int get_dist(const int target_id);

    int get_prev_id(const int target_id);

    int get_id(void);

    int get_total_node_num(void);

    void set_dist(const int target_id, const int dist);

    void set_prev_id(const int of_what, const int id);

    void set_db(database* db);

    void run_dijkstra(void);

    void send(node &neighbor, LSP_t &lsp);

    void queue_packet(LSP_t &lsp);

    LSP_t make_packet(const int, const int);

    void request(void);

    void construct_table(void);

    void print_routing_table(FILE* file);

    std::string send_msg(const char* message, const int dest_id);

    void add_prev(const int target_id, const int prev_id);

    void init_dist(void);

    void apply_inf_edge(const int v1, const int v2);

    static int get_dijkstra_ready_id(void) {
        return node::db->pop_dijkstra_queue();
    }

    static bool is_dijkstra_queue_empty(void) {
        return node::db->is_queue_empty();
    }

    static void push_dijkstra_queue(int target_id) {
        node::db->wait_dijkstra(target_id);
    }

    static void update_edge_num(const int edge_num) {
        node::db->set_edge_num(edge_num);
    }

    static void append_nbd(const int to_what, const int nbd_id) {
        node::db->add_nbd(to_what, nbd_id);
    }

};

database* node::db;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        print(usage: linkstate topologyfile messagesfile changesfile);
    }
    FILE* topology = fopen((const char*)argv[TOP_PATH], "r");
    FILE* msg = fopen((const char*)argv[MSG_PATH], "r");
    FILE* changes = fopen((const char*)argv[CHANGES_PATH], "r");
    FILE* output = fopen("output_ls.txt", "w");

    if (topology == NULL || msg == NULL || changes == NULL) {
        print(Error: open input file.);
        exit(ENOENT);
    }

    int router_num = 0;

    int _num = fscanf(topology, "%d", &router_num);

    database* db = new database(router_num);

    std::vector<node> routers;
    for(int i = 0 ; i < router_num; i++) {
        node n(i, router_num);
        routers.push_back(n);
    }
    routers[0].set_db(db);

    int from = 0, to = 0, cost = 0, e_num = 0;

    while(fscanf(topology, "%d %d %d", &from, &to, &cost) != EOF) {
        LSP_t p = routers[from].make_packet(to, cost);
        routers[from].send(routers[to], p);
        LSP_t q = routers[to].make_packet(from, cost);
        routers[to].send(routers[from], q);
        node::append_nbd(from, to);
        node::append_nbd(to, from);
        e_num++;
    }


    for(int i = 0 ; i < router_num; i++) {
        routers[i].request();
    }

    while(!node::is_dijkstra_queue_empty()) {
        int target_id = node::get_dijkstra_ready_id();
        routers[target_id].run_dijkstra();
    }

    for(int i = 0; i < router_num; i++) {
        routers[i].construct_table();
        routers[i].print_routing_table(output);
        _num = fprintf(output, "\n");
    }

    std::vector<std::string> raw_msgs;
    std::vector<std::pair<int, int> > src_dest_msg;
    char* raw = (char*)malloc(sizeof(char) * 1010);
    char* a_msg = (char*)malloc(sizeof(char) * 1010);

    while(true) {
        memset(raw, 0, 1010);
        fgets(raw, 1010, msg);
        int src_id = 0, dest_id = 0;
        _num = sscanf(raw, "%d %d %[^\n]", &src_id, &dest_id, a_msg);
        if(_num < 3 || _num == EOF) {
            break;
        }
        raw_msgs.push_back(std::string(a_msg));
        src_dest_msg.push_back(std::pair<int, int>(src_id, dest_id));
    }

    free(raw);
    raw = NULL;
    free(a_msg);
    a_msg = NULL;

    for(size_t i = 0; i < src_dest_msg.size(); i++) {
        std::string message = routers[src_dest_msg[i].first].send_msg(raw_msgs[i].c_str(), src_dest_msg[i].second);
        _num = fprintf(output, "%s\n", message.c_str());
    }

    _num = fprintf(output, "\n");

    while(fscanf(changes, "%d %d %d", &from, &to, &cost) != EOF) {
        if(cost < 0) {
            LSP_t p1 = routers[from].make_packet(to, INF);
            LSP_t p2 = routers[to].make_packet(from, INF);
            routers[from].send(routers[to], p1);
            routers[to].send(routers[from], p2);
        }
        else {
            LSP_t p1 = routers[from].make_packet(to, cost);
            LSP_t p2 = routers[to].make_packet(from, cost);
            routers[from].send(routers[to], p1);
            routers[to].send(routers[from], p2);
        }
        for(int i = 0 ; i < router_num; i++) {
            routers[i].request();
        }
        while(!node::is_dijkstra_queue_empty()) {
            int target_id = node::get_dijkstra_ready_id();
            if (cost < 0) {
                routers[target_id].apply_inf_edge(from, to);
            }
            routers[target_id].run_dijkstra();
        }

        for(int i =0 ; i < router_num; i++) {
            routers[i].construct_table();
            routers[i].print_routing_table(output);
            _num = fprintf(output, "\n");
        }

        for(size_t i = 0 ; i < src_dest_msg.size(); i++) {
            std::string message = routers[src_dest_msg[i].first].send_msg(raw_msgs[i].c_str(), src_dest_msg[i].second);
            _num = fprintf(output, "%s\n", message.c_str());
        }

        _num = fprintf(output, "\n");
    }

    /* everything is successfully done! */
    print(Complete.Output file written to output_ls.txt.);

    /* clearing */
    delete db;
    fclose(topology);
    fclose(msg);
    fclose(changes);
    fclose(output);
    return 0;
}

database::database(int router_num) {
    this->router_num = router_num;
    for (int i = 0; i < router_num; i++) {
        this->seq_nums.push_back(0);
    }
    for (int i = 0; i < router_num; i++) {
        std::vector<int> line(router_num, INF);
        this->edge_cost.push_back(line);
        this->edge_cost[i][i] = 0;
    }
    this->edge_num = 0;  // 명목상 초기화 하는것
}

std::vector<int> &database::get_nbds(const int id) {
    return this->nbds[id];
}

int database::get_edge_cost(const int id1, const int id2) {
    return this->edge_cost[id1][id2];
}

int database::get_router_num(void) {
    return this->router_num;
}

int database::get_edge_num(void) {
    return this->edge_num;
}

void database::set_edge_cost(const int id1, const int id2, const int cost) {
    this->edge_cost[id1][id2] = cost;
    this->edge_cost[id2][id1] = cost;
}

void database::add_nbd(const int to_what, const int nbd_id) {
    if (this->nbds.find(to_what) == this->nbds.end()) {
        std::vector<int> n(1, nbd_id);
        this->nbds[to_what] = n;
    } else {
        this->nbds[to_what].push_back(nbd_id);
    }
}

void database::remove_nbd(const int from_what, const int target_id) {
    std::vector<int> &n = this->nbds[from_what];
    for (size_t i = 0; i < n.size(); i++) {
        if (n[i] == target_id) {
            n.erase(n.begin() + i);
            break;
        }
    }
}

int database::get_seq_num(int target_id) {
    return this->seq_nums[target_id];
}

void database::increase_seq_num(int target_id) {
    this->seq_nums[target_id]++;
}

void database::wait_dijkstra(int target_id) {
    this->dijkstra_queue.push(target_id);
}

int database::pop_dijkstra_queue(void) {
    int id = this->dijkstra_queue.front();
    this->dijkstra_queue.pop();
    return id;
}

bool database::is_nbd(const int src, const int target) {
    std::vector<int>& nbds = this->get_nbds(src);
    for(int nbd : nbds) {
        if (nbd == target) {
            return true;
        }
    }
    return false;
}

bool database::is_queue_empty(void) {
    return this->dijkstra_queue.empty();
}

void database::set_edge_num(const int edge_num) {
    this->edge_num = edge_num;
}

node::node(const int id, const int total_node_num) {
    this->id = id;
    this->exists_change = false;
    this->total_node_num = total_node_num;
    for (int i = 0; i < total_node_num; i++) {
        this->dist.push_back(INF);
        this->prev.push_back(-1);
        entry_t e = { INF, -1 };
        this->routing_table.push_back(e);
    }
    this->dist[id] = 0;
    this->prev[id] = id;
    this->routing_table[this->id] = { 0, this->id };
}

int node::get_dist(const int target_id) {
    return this->dist[target_id];
}

int node::get_prev_id(const int target_id) {
    return this->prev[target_id];
}

int node::get_id(void) {
    return this->id;
}

int node::get_total_node_num(void) {
    return this->total_node_num;
}

void node::set_dist(const int target_id, const int dist) {
    this->dist[target_id] = dist;
}

void node::set_prev_id(const int of_what, const int id) {
    this->prev[of_what] = id;
}

void node::set_db(database* db) {
    this->db = db;
}

void node::run_dijkstra(void) {
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int> >, q_cmp> open;
    std::set<int> discovered;
    open.push(std::pair<int, int>(0, this->id));
    discovered.insert(this->id);
    this->exists_change = false;
    while (!open.empty()) {
        std::pair<int, int> cur = open.top();
        open.pop();
        int cur_id = cur.second;
        int to_cur_dist = this->dist[cur_id];
        std::vector<int> &nbds = this->db->get_nbds(cur_id);
        for (int nbd_id: nbds) {
            int to_nbd_dist = this->dist[nbd_id];
            int new_nbd_dist = add_inf(to_cur_dist, this->db->get_edge_cost(cur_id, nbd_id));
            if (to_nbd_dist == new_nbd_dist && this->prev[nbd_id] != -1){
                this->prev[nbd_id] = min(this->prev[nbd_id], cur_id);
            }
            if (left_is_less_inf(new_nbd_dist, to_nbd_dist)) {
                this->exists_change = true;
                this->prev[nbd_id] = cur_id;
                this->dist[nbd_id] = new_nbd_dist;
                open.push(std::pair<int, int>(new_nbd_dist, nbd_id));
            }
            if (discovered.find(nbd_id) == discovered.end()) {
                open.push(std::pair<int, int>(new_nbd_dist, nbd_id));
                discovered.insert(nbd_id);
            }
        }
    }
    if(this->exists_change) {
        std::vector<int>& nbds = this->db->get_nbds(this->id);
        for(int nbd : nbds) {
            this->db->wait_dijkstra(nbd);
        }
    }
}

void node::send(node &neighbor, LSP_t &lsp) {
    neighbor.queue_packet(lsp);
}

void node::queue_packet(LSP_t &lsp) {
    int valid_seq_num = this->db->get_seq_num(lsp.sender_id) - 1;
    if(valid_seq_num == lsp.sequence_num)
        this->packet_received.push(lsp);
}

LSP_t node::make_packet(const int id, const int cost) {
    LSP_t lsp;
    lsp.sender_id = this->id;
    lsp.sequence_num = this->db->get_seq_num(this->id);
    this->db->increase_seq_num(this->id);
    lsp.id_dist.push_back(std::pair<int, int>(id, cost));
    return lsp;
}

void node::request(void) {
    while (!this->packet_received.empty()) {
        LSP_t& packet = this->packet_received.front();
        this->db->set_edge_cost(packet.id_dist[0].first, packet.sender_id, packet.id_dist[0].second);
        if(packet.id_dist[0].second == INF) {
            
            this->db->remove_nbd(packet.id_dist[0].first, packet.sender_id);
        }
        else {
            if(!this->db->is_nbd(this->id, packet.sender_id)) {
                this->db->add_nbd(this->id, packet.sender_id);
            }
        }
        //this->dist[packet.sender_id] = packet.id_dist[0].second;
        this->db->wait_dijkstra(this->id);
        this->packet_received.pop();
    }
}

void node::construct_table(void) {
    for(int i = 0; i < this->total_node_num; i++) {
        this->routing_table[i].cost = this->dist[i];
    }
    for (int i = 0; i < this->total_node_num; i++) {
        // next[i]를 구하는게 목적
        int idx = i;
        while (true) {
            if (this->prev[idx] == this->id) {
                this->routing_table[i].next = idx;
                break;  // 현재 index의 이전이 도착 지점이기 때문에 현재 index가 바로 next가 된다.
            }
            idx = this->prev[idx];
        }
    }
}

void node::print_routing_table(FILE* file) {
    for(size_t i =0 ; i < this->routing_table.size(); i++) {
        int _num = fprintf(file, "%zu %d %d\n", i, this->routing_table[i].next, this->routing_table[i].cost);
    }
}

std::string node::send_msg(const char *message, const int dest_id) {
    std::string raw_msg(message);
    if(this->dist[dest_id] == INF) {
        return std::string("from ") + std::to_string(this->id) + " to " + std::to_string(dest_id) + " cost infinite hops unreachable message " + raw_msg;
    }
    std::string path("");
    std::string msg("");

    int prev_id = this->get_prev_id(dest_id);

    while(true) {
        path = std::to_string(prev_id) + " " + path;
        if(prev_id == this->id) {
            break;
        }
        prev_id = this->get_prev_id(prev_id);
    }
    msg = "from " + std::to_string(this->id) + " to " + std::to_string(dest_id) + " cost " + std::to_string(this->dist[dest_id]) + " hops " + path + "message " + raw_msg;
    return msg;
}

void node::add_prev(const int target_id, const int prev_id) {
    this->prev[target_id] = prev_id;
}

void node::init_dist(void) {
    for (size_t i = 0; i < this->dist.size(); i++) {
        this->dist[i] = INF;
    }
    this->dist[this->id] = 0;
}

void node::apply_inf_edge(const int v1, const int v2) {
    for (int dest = 0; dest < this->total_node_num; dest++) {
        std::vector<int> path;
        int idx = dest;
        path.push_back(idx);
        while (this->prev[idx] != this->id) {
            idx = this->prev[idx];
            path.push_back(idx);
        }
        path.push_back(this->prev[idx]);
        for (size_t i = 0; i < path.size() - 1; i++) {
            if ((path[i] == v1 && path[i + 1] == v2) || (path[i] == v2 && path[i + 1] == v1)) {
                this->dist[dest] = INF;
                break;
            }
        }
    }
}



int add_inf(const int a, const int b) {
    switch (a) {
        case INF:
            return INF;
            // intentional fallthrough
        default:
            switch (b) {
                case INF:
                    return INF;
                    // intentional fallthrough
                default:
                    return a + b;
            }
    }
    assert(false);
}

bool left_is_less_inf(const int a, const int b) {
    switch (a) {
        case INF:
            return false;
            // intentional fallthrough
        default:
            switch (b) {
                case INF:
                    return true;
                    // intentional fallthrough
                default:
                    return a < b;
            }
    }
    assert(false);
}
template<typename T>
T min(T a, T b) {
    if (a > b) {
        return b;
    }
    return a;
}

template<typename T>
T max(T a, T b) {
    if (a > b) {
        return a;
    }
    return b;
}
