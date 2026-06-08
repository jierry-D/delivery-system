#pragma once
#include "Graph.h"
#include "DynArray.h"
#include "HashMap.h"

struct PathResult {
    DynArray<int> path;
    double totalTime  = 0;
    double totalCost  = 0;
    bool   reachable  = false;
};

class Dijkstra {
public:
    // 单源最短耗时：返回从 src 到所有节点的结果
    static HashMap<int, PathResult> shortestTimeFrom(const Graph& g, int src);

    // 两点最低费用路径
    static PathResult cheapestPath(const Graph& g, int src, int dst);
};
