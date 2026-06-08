#pragma once
#include "Graph.h"
#include <vector>
#include <unordered_map>

struct TopoResult {
    bool hasCycle;
    std::vector<int> order;       // 拓扑排序后的配送顺序
    std::vector<int> cycleNodes;  // 有环时标记涉及节点
};

class TopoSort {
public:
    // 对指定节点子集进行拓扑排序（Kahn BFS 算法）
    static TopoResult sort(const Graph& g, const std::vector<int>& nodeIds);

private:
    static std::unordered_map<int, int> buildInDegree(
        const Graph& g,
        const std::vector<int>& nodeIds,
        const std::unordered_map<int, bool>& inSet);
};
