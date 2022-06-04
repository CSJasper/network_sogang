# Sogang Network Assignment Master


## Hw1




## Hw2


### methods

1. 초기 cost는 전부 정수가 가질수 있는 최댓값으로 초기화 한다.

#### update_table()

* call_queue는 반드시 나로부터 다른 모든 이웃한 이웃으로 table을 보내고 그 table을 바탕으로 이웃의 table을 업데이트 하는 과정을 나타낸 것이다.
* 그렇게 업데이트 된 이웃의 테이블은 그 자신이 source가 되어 이 함수를 호출함으로써 table을 이웃한 이웃에게 전달한다.
* 만약 그래프의 topology가 변경되어서 이 함수가 호출된다는 것은 변경된 정점을 source로 하여 그것들의 이웃한 정점의 table을 업데이트 하는 과정을 수행한다.

1. 이 함수는 graph 구조가 초기화 되었고 distance vector table이 초기화 되었다는 전제를 가지고 시작한다. 즉, 각 라우터에 대하여 유한 번의 탐색을 통하여 다른 라우터까지의 연결 상태를 알 수 있다.
2. 이 함수는 call_queue에 있는 source와 direct neighbor에 대해서만 routing table의 업데이트를 진행한다.
3. 이 힘수는 만약 graph state의 변화가 일어나면 변화가 일어난 vertex의 routing table을 업데이트 하는데, 이때 해당 vertex의 기존 path vector의 유효성을 검사해야 한다.
	*	path vector는 초기에 size가 0인 상태로 시작한다.
	*	path vector는 계속해서 이어지는 방식으로 만들어진다.
	* 	path vector의 size가 0이라는 것은 연결되지 않았다는 의미이다.
	*	path vector의 유효성은 graph state에 변화가 일어난 경우 중, edge가 disconnect 되었을 때 해당 edge를 구성하는 vertices가 만약 path vector에 포함되었을 경우 새로운 그 destination 까지의 거리를 infinite로 변경해야 한다.
4. routing table을 업데이트 할때, direct nbd의 table을 훑으면서 source를 거쳐서 갈수 있는 destination이라면 기존의 cost와 비교해서 작다면 갱신한다. 갱신시에는 next를 source로 하고 해당 path vector를 append해서 path vector도 갱신한다.
 -> routing table을 훑으면서 현재의 cost를 저장 source 중에서 해당 목적지로 가는 버텍스중 cost가 최소이고 id값이 작은 버텍스의 cost 저장해서 비교
 -> 상황 정리
 1. table[i][j] -> ith 노드에서 jth 노드로 가는 cost와 next 노드가 저장되어 있다. 이때 cost는 처음에는 `INT_MAX`로 초기화 되어있다. next는 -1로 초기화 된다. (For i != j)
 2. table[i][j] -> 자기자신으로 가는 정보가 담겨있다. cost는 0으로 초기화되고, next는 -1로 초기화 된다.
 3. table[i][j](이웃의 table)를 업데이트 하는 것이 목표 -> table[i][j]는 기존의 cost와 table[source_id][j]의 cost + dist(source, direct_nbd)를 비교해서 더 작다면 갱신
 4. 단 업데이트 할 때, 여러가지 source_id를 보면서 cost를 우선으로 업데이트 하고 cost가 같다면 source_id가 작은 순서로 업데이트 한다.
 5. 그렇기 때문에 queue에서 원소를 빼는 과정에서 동시에 queue에 원소를 삽입하면 안된다. 즉, 따로 저장해 놓고 queue가 비어있을때 따로 저장해 놓은 것을 삽입하면 된다. (double queue로 해결) (우선순위 큐로 구현하면 더 좋을 듯?->이미 했으니 나중에 시간나면 수정)
 6. cost를 비교하는데 있어서는 전부 특수한 연산으로 처리 해야한다.(오버플로우)
5. 만약 cost가 같은 노드라면 id값이 작은 노드로 선택하여 path를 정한다. 즉 cost가 최소인걸로 바로 정하지 말고 다른 이웃한 노드를 전부 보면서 비교한 후 최종 선택을 해야 한다는 것이다. 즉 direct nbd가 source에서 routing table을 받았을 때, (특정 라우터로 가는 경로가 아직 없다면 그걸로 잡고, 있다면 cost가 작은것)으로 cost도 같다(이때는 path가 완전한 path인지 확인해 봐야 한다.)면 현재 있는 path와 비교했을 때, path의 order가 더 작은 path로 경로를 정한다.
6. 만약 cost가 update 되었다면 direct_nbd의 routing table이 변경되었다는 의미이므로 direct_nbd의 nbd를 call_queue에 넣는다.

#### path vector의 유효성을 검사해야하는 경우 -> 유효성 검사 함수를 구현해야 한다.
 1. 다른 정점으로부터의 routing table을 받아 나의 routing table을 업데이트 하는 경우 : cost를 갱신할 때 현재의 최소의 cost가 나오게 된 경로가 유효한지 확인한다.
 2. 만약 pathvector의 마지막 부분에 destination이 없다면 path vector가 초기화되지 않은 경우이므로 유효성 검사를 하지 않는다.
 
 
 #### 그냥 노드마다 메시지를 저장하는 큐를 만들까?
 
 
 
 1. graph에서는 node를 배열의 형태로 저장한다.
 2. routing table은 node class의 attribute로 정한다.
 3. graph class는 graph를 이루는 node를 초기화하고 graph에 대한 정보를 저장한다.
 4. graph class는 topology가 바뀔 때 마다 이를 node class에게 알리는 역할을 한다.
 5. 통신은 node간 통신이다. graph는 alarm 같은 기능
 
 
 처음에 파일을 읽으면서 노드를 초기화 -> 처음보는 id의 노드라면 새롭게 만들고 routing table을 업데이트 한다. 이전에 만들었던 노드라면 routing table만 업데이트 한다. (즉 노드의 id로 해시 테이블을 만들어 저장하는 것이 좋다)
 
 
 ### 초기 상태로
 1. 파일을 읽으면서 그래프 노드 상태 업데이트
 2. 의사코드
 [This is an algorithm for class graph]
 for (each node transfer its routing table to its neighborhood)
	for each node check whether the message queue is not empty
		if message queue is not empty -> update its routing table (update_node)
		if message queue is empty -> do nothing (update_node)
	
3. routing table을 업데이트 하는 의사코드(for single node)
	이 메서드는 메시지 큐가 비어있지 않다는 것을 전제로 호출된다.
	메시지 큐를 하나씩 팝하면서 자신의 라우팅 테이블을 업데이트 하는데
	현재의 라우팅 테이블을 보면서 목적지 까지의 거리와 메시지 큐에서 팝한 라우팅 테이블을 보면서
	(팝한 라우터 까지의 거리) + (그 라우터에서 목적지 까지의 거리) 가 더 짧다면 최소 거리를 업데이트 하고
	(팝한 라우터의 id)를 따로 저장한다.
	만약 (팝한 라우터 까지의 거리) + (그 라우터에서 목적지 까지의 거리)가 최소와 같다면
	(팝 한 라우터의 id)와 기존 최소가 되게 하는 next router id와 비교해서 더 작은 id를 다음 라우터로 저장한다.
	
의사코드
cur_routing_table = assign table()
 while msg queue is not empty
	nbd_id, nbd_tbl = msg_queue_pop()
	nbd_cost = cur_routing_table[nbd_id].cost
	for i is index of current routing table
		cur_cost = cur_routing_table[].cost
		if (cur_cost > nbd_tbl[i] + nbd_cost)
			cur_routing_table[i].cost = nbd_tbl[i] + nbd_cost;
			cur_routing_table[i].next = nbd_id;
		else if (cur_cost == nbd_tbl[i] + nbd_cost)
			cur_routing_table[i].next = min(nbd_id, cur_routing_table[i].next)
			
			
4. path vector의 처음부터 끝까지.
	1. node가 생성될 때 모든 routing table의 path vector는 전부 자기 자신의 id로 push_back() 한다. -> graph의 생성자에서 한다.
	2. path vector를 채워 넣을 때, 인접한 이웃 노드로 가는 path vector의 경우에는 걍 이웃 노드의 id를 걍 push_back()하면 된다. -> 걍 3번과 같은거 하면 됨
	3. 만약 인접한 이웃 노드가 아닌 노드로 가는 path vector의 경우에는 다른 라우팅 테이블의 path vector를 가져다가 붙이면 된다. (자기 자신 + 다른 라우팅 테이블의 path vector를 가져다가 붙인다. / 자기 자신의 path vector는 날려버려야댐)
	4. 만약 어떤 edge가 끊어져서 경로가 달라지는 경우에는 그 edge에 포함되는 어떤 한 노드의 routing table에서 다른 한 노드로 가는 path를 없애버린다 -> 자기 자신만 있도록 초기 graph::update_state에서 한다.
	5. 만약 어떤 edge의 cost가 변경되는 경우에도 그 edge를 구성하는 노드들의 routing table을 변경해야 한다. -> 
	6. 만약 edge의 상태가 변경된다면 이 알고리즘은 edge를 구성하는 노드들만 상태를 변경한다. 그러나 기존의 이 edge를 통과하는 최단경로를 가지는 노드들은 이 edge가 없어졌는지 모른다. 그러므로 edge 정보가 변경되면
	그 edge를 구성하는 노드들은 자신의 이웃 노드들에게 변경된 걸로 cost를 수정하라고 해야됨
	
	
linked state 알고리즘

클래스 설계

State the requirement of linked stated algorithm
1. graph에서의 노드는 각 지점에서의 라우터들을 의미한다.
2. 라우터들은 각각의 LS