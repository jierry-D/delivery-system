#include "../include/Dijkstra.h"
#include "../include/MinHeap.h"
#include "../include/Logger.h"
#include <limits>
#include <string>

using std::to_string;
static const double INF = std::numeric_limits<double>::infinity();

// 堆节点
struct DijkEntry { double dist; int node; };

// 最小堆比较器（dist 小的优先级高）
struct DijkCmp {
    bool operator()(const DijkEntry& a, const DijkEntry& b) const {
        return a.dist < b.dist;
    }
};

// ── 工具：从 prev 表回溯路径 ──────────────────────────
static DynArray<int> buildPath(const HashMap<int,int>& prev, int dst) {
    DynArray<int> path;
    for (int cur = dst; cur != -1; ) {
        path.push_back(cur);
        const int* p = prev.find(cur);
        cur = p ? *p : -1;
    }
    // 原地反转
    for (int l = 0, r = path.size() - 1; l < r; ++l, --r) {
        int t = path[l]; path[l] = path[r]; path[r] = t;
    }
    return path;
}

// ── 单源最短耗时 ──────────────────────────────────────
HashMap<int, PathResult> Dijkstra::shortestTimeFrom(const Graph& g, int src) {
    if (!g.hasNode(src)) {
        LOG_ERROR("Dijkstra 起点 " + to_string(src) + " 不存在");
        return HashMap<int, PathResult>{};
    }

    DynArray<int> ids = g.getAllNodeIds();

    HashMap<int, double> dist, costAccum;
    HashMap<int, int>    prev;
    for (int i = 0; i < ids.size(); ++i) {
        dist[ids[i]]     = INF;
        costAccum[ids[i]] = INF;
        prev[ids[i]]     = -1;
    }
    dist[src]     = 0.0;
    costAccum[src] = 0.0;

    MinHeap<DijkEntry, DijkCmp> pq;
    pq.push({0.0, src});

    while (!pq.empty()) {
        DijkEntry top = pq.top(); pq.pop();
        double d = top.dist;
        int    u = top.node;
        double* dp = dist.find(u);
        if (!dp || d > *dp) continue;

        const DynArray<Edge>& nbrs = g.getNeighbors(u);
        for (int i = 0; i < nbrs.size(); ++i) {
            int    v  = nbrs[i].to;
            double nd = *dp + nbrs[i].time;
            double* dv = dist.find(v);
            if (!dv) continue;
            if (nd < *dv) {
                *dv = nd;
                double* cv = costAccum.find(v);
                double* cu = costAccum.find(u);
                if (cv && cu) *cv = *cu + nbrs[i].cost;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }

    HashMap<int, PathResult> results;
    for (int i = 0; i < ids.size(); ++i) {
        int id = ids[i];
        PathResult pr;
        double* dv = dist.find(id);
        pr.reachable = (dv && *dv != INF);
        if (pr.reachable) {
            pr.totalTime = *dv;
            double* cv = costAccum.find(id);
            pr.totalCost = cv ? *cv : 0.0;
            pr.path = buildPath(prev, id);
        }
        results.set(id, pr);
    }
    return results;
}

// ── 两点最低费用 ──────────────────────────────────────
PathResult Dijkstra::cheapestPath(const Graph& g, int src, int dst) {
    if (!g.hasNode(src) || !g.hasNode(dst)) {
        LOG_ERROR("cheapestPath：起点或终点不存在");
        return PathResult{};
    }

    DynArray<int> ids = g.getAllNodeIds();

    HashMap<int, double> dist, timeAccum;
    HashMap<int, int>    prev;
    for (int i = 0; i < ids.size(); ++i) {
        dist[ids[i]]     = INF;
        timeAccum[ids[i]] = INF;
        prev[ids[i]]     = -1;
    }
    dist[src]     = 0.0;
    timeAccum[src] = 0.0;

    MinHeap<DijkEntry, DijkCmp> pq;
    pq.push({0.0, src});

    while (!pq.empty()) {
        DijkEntry top = pq.top(); pq.pop();
        double d = top.dist;
        int    u = top.node;
        double* dp = dist.find(u);
        if (!dp || d > *dp) continue;

        const DynArray<Edge>& nbrs = g.getNeighbors(u);
        for (int i = 0; i < nbrs.size(); ++i) {
            int    v  = nbrs[i].to;
            double nd = *dp + nbrs[i].cost;  // 按费用
            double* dv = dist.find(v);
            if (!dv) continue;
            if (nd < *dv) {
                *dv = nd;
                double* tv = timeAccum.find(v);
                double* tu = timeAccum.find(u);
                if (tv && tu) *tv = *tu + nbrs[i].time;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }

    PathResult pr;
    double* dv = dist.find(dst);
    pr.reachable = (dv && *dv != INF);
    if (pr.reachable) {
        pr.totalCost = *dv;
        double* tv = timeAccum.find(dst);
        pr.totalTime = tv ? *tv : 0.0;
        pr.path = buildPath(prev, dst);
    }
    return pr;
}
