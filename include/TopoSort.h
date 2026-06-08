#pragma once
#include "Graph.h"

struct TopoResult {
    bool          hasCycle=false;
    DynArray<int> order;       // 拓扑排序顺序
    DynArray<int> cycleNodes;  // 环路涉及节点
};

// Kahn BFS 拓扑排序（纯头文件实现，inline 避免多编译单元重定义）
class TopoSort {
public:
    static inline TopoResult sort(const Graph& g, const DynArray<int>& ids) {
        TopoResult res;
        if (ids.empty()) return res;

        HashMap<int,bool> inSet;
        for (int i=0; i<ids.size(); ++i) inSet.set(ids[i], true);

        // 计算入度
        HashMap<int,int> deg;
        for (int i=0; i<ids.size(); ++i) deg[ids[i]]=0;
        for (int i=0; i<ids.size(); ++i)
            for (const Edge& e : g.getNeighbors(ids[i]))
                if (inSet.contains(e.to)) ++deg[e.to];

        Queue<int> q;
        for (int i=0; i<ids.size(); ++i)
            if (deg[ids[i]]==0) q.push(ids[i]);

        while (!q.empty()) {
            int u=q.front(); q.pop();
            res.order.push_back(u);
            for (const Edge& e : g.getNeighbors(u))
                if (inSet.contains(e.to) && --deg[e.to]==0) q.push(e.to);
        }

        if (res.order.size() < ids.size()) {
            res.hasCycle=true;
            HashMap<int,bool> done;
            for (int i=0; i<res.order.size(); ++i) done.set(res.order[i],true);
            for (int i=0; i<ids.size(); ++i)
                if (!done.contains(ids[i])) res.cycleNodes.push_back(ids[i]);
        }
        return res;
    }
};
