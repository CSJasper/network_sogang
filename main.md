# Sogang Network Assignment Master


## Hw1




## Hw2


### methods

1. 초기 cost는 전부 정수가 가질수 있는 최댓값으로 초기화 한다.
#### update_table()
1. 이 함수는 graph 구조가 초기화 되었고 distance vector table이 초기화 되었다는 전제를 가지고 시작한다. 즉, 각 라우터에 대하여 유한 번의 탐색을 통하여 다른 라우터까지의 연결 상태를 알 수 있다.
2. 이 함수는 call_queue에 있는 source와 direct neighbor에 대해서만 routing table의 업데이트를 진행한다.
3. 이 힘수는 만약 graph state의 변화가 일어나면 변화가 일어난 vertex의 routing table을 업데이트 하는데, 이때 해당 vertex의 기존 path vector의 유효성을 검사해야 한다.
	*	path vector는 초기에 size가 0인 상태로 시작한다.
	*	path vector는 계속해서 이어지는 방식으로 만들어진다.
	* 	path vector의 size가 0이라는 것은 연결되지 않았다는 의미이다.
	*	path vector의 유효성은 graph state에 변화가 일어난 경우 중, edge가 disconnect 되었을 때 해당 edge를 구성하는 vertices가 만약 path vector에 포함되었을 경우 새로운 그 destination 까지의 거리를 infinite로 변경해야 한다.
4. routing table을 업데이트 할때, direct nbd의 table을 훑으면서 source를 거쳐서 갈수 있는 destination이라면 기존의 cost와 비교해서 작다면 갱신한다. 갱신시에는 next를 source로 하고 해당 path vector를 append해서 path vector도 갱신한다.
5. 만약 cost가 같은 노드라면 id값이 작은 노드로 선택하여 path를 정한다. 즉 cost가 최소인걸로 바로 정하지 말고 다른 이웃한 노드를 전부 보면서 비교한 후 최종 선택을 해야 한다는 것이다.
6. 