#include "../include/TopoSort.h"
#include "../include/Queue.h"
#include "../include/Logger.h"
#include <string>

using std::to_string;

HashMap<int, int> TopoSort::buildInDegree(
    const Graph&          g,
    const DynArray<int>&  nodeIds,
    const HashMap<int, bool>& inSet)
{
    HashMap<int, int> inDeg;
    for (int i = 0; i < nodeIds.size(); ++i)
        inDeg[nodeIds[i]] = 0;

    for (int i = 0; i < nodeIds.size(); ++i) {
        const DynArray<Edge>& nbrs = g.getNeighbors(nodeIds[i]);
        for (int j = 0; j < nbrs.size(); ++j) {
            if (inSet.contains(nbrs[j].to)) {
                int* deg = inDeg.find(nbrs[j].to);
                if (deg) ++(*deg);
            }
        }
    }
    return inDeg;
}

TopoResult TopoSort::sort(const Graph& g, const DynArray<int>& nodeIds) {
    TopoResult result;

    if (nodeIds.empty()) return result;

    // 构建节点集合
    HashMap<int, bool> inSet;
    for (int i = 0; i < nodeIds.size(); ++i)
        inSet.set(nodeIds[i], true);

    HashMap<int, int> inDeg = buildInDegree(g, nodeIds, inSet);

    Queue<int> q;
    for (int i = 0; i < nodeIds.size(); ++i) {
        int* d = inDeg.find(nodeIds[i]);
        if (d && *d == 0) q.push(nodeIds[i]);
    }

    while (!q.empty()) {
        int u = q.front(); q.pop();
        result.order.push_back(u);

        const DynArray<Edge>& nbrs = g.getNeighbors(u);
        for (int i = 0; i < nbrs.size(); ++i) {
            int v = nbrs[i].to;
            if (!inSet.contains(v)) continue;
            int* d = inDeg.find(v);
            if (d) {
                --(*d);
                if (*d == 0) q.push(v);
            }
        }
    }

    // 排序结果数量 < 节点数 → 有环
    if (result.order.size() < nodeIds.size()) {
        result.hasCycle = true;
        // 标记未被处理的节点为环路节点
        HashMap<int, bool> processed;
        for (int i = 0; i < result.order.size(); ++i)
            processed.set(result.order[i], true);
        for (int i = 0; i < nodeIds.size(); ++i)
            if (!processed.contains(nodeIds[i]))
                result.cycleNodes.push_back(nodeIds[i]);
        LOG_WARN("拓扑排序检测到环路，涉及节点数：" +
                 to_string(result.cycleNodes.size()));
    } else {
        LOG_INFO("拓扑排序完成，共 " + to_string(result.order.size()) + " 个节点");
    }
    return result;
}
