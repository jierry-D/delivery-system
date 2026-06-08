#pragma once
#include "Graph.h"
#include <vector>
#include <map>

struct PathResult {
    std::vector<int> path;  // 节点序列
    double totalTime;
    double totalCost;
    bool reachable;

    PathResult() : totalTime(0), totalCost(0), reachable(false) {}
};

class Dijkstra {
public:
    // 单源最短耗时，返回从 src 到所有节点的结果
    static std::map<int, PathResult> shortestTimeFrom(const Graph& g, int src);

    // 两点最低费用路径
    static PathResult cheapestPath(const Graph& g, int src, int dst);
};
