#include "../include/TopoSort.h"
#include "../include/Logger.h"
#include <queue>
#include <unordered_set>

using namespace std;

unordered_map<int, int> TopoSort::buildInDegree(
    const Graph& g,
    const vector<int>& nodeIds,
    const unordered_map<int, bool>& inSet)
{
    unordered_map<int, int> inDegree;
    for (int id : nodeIds) inDegree[id] = 0;

    for (int from : nodeIds) {
        for (const Edge& e : g.getNeighbors(from)) {
            if (inSet.count(e.to)) {
                inDegree[e.to]++;
            }
        }
    }
    return inDegree;
}

TopoResult TopoSort::sort(const Graph& g, const vector<int>& nodeIds) {
    TopoResult result;
    result.hasCycle = false;

    if (nodeIds.empty()) return result;

    // 构建节点集合
    unordered_map<int, bool> inSet;
    for (int id : nodeIds) inSet[id] = true;

    auto inDegree = buildInDegree(g, nodeIds, inSet);

    queue<int> q;
    for (int id : nodeIds) {
        if (inDegree[id] == 0) q.push(id);
    }

    while (!q.empty()) {
        int u = q.front(); q.pop();
        result.order.push_back(u);

        for (const Edge& e : g.getNeighbors(u)) {
            if (!inSet.count(e.to)) continue;
            inDegree[e.to]--;
            if (inDegree[e.to] == 0) q.push(e.to);
        }
    }

    // 若排序结果数量小于节点数，说明存在环
    if ((int)result.order.size() < (int)nodeIds.size()) {
        result.hasCycle = true;
        // 收集未被处理的节点（环路节点）
        unordered_set<int> processed(result.order.begin(), result.order.end());
        for (int id : nodeIds) {
            if (!processed.count(id)) result.cycleNodes.push_back(id);
        }
        LOG_WARN("拓扑排序检测到环路，涉及节点数：" +
                 to_string(result.cycleNodes.size()));
    } else {
        LOG_INFO("拓扑排序完成，共 " + to_string(result.order.size()) + " 个节点");
    }
    return result;
}
