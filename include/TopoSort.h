#pragma once
#include "Graph.h"
#include "DynArray.h"
#include "HashMap.h"

struct TopoResult {
    bool          hasCycle  = false;
    DynArray<int> order;       // 拓扑排序后的配送顺序
    DynArray<int> cycleNodes;  // 有环时标记涉及节点
};

class TopoSort {
public:
    // Kahn BFS 拓扑排序，对指定节点子集操作
    static TopoResult sort(const Graph& g, const DynArray<int>& nodeIds);

private:
    static HashMap<int, int> buildInDegree(
        const Graph&        g,
        const DynArray<int>& nodeIds,
        const HashMap<int, bool>& inSet);
};
