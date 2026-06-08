#include "../include/Dijkstra.h"
#include "../include/Logger.h"
#include <limits>

static const double INF = std::numeric_limits<double>::infinity();

struct DE { double d; int n; };
struct DECmp { bool operator()(const DE& a, const DE& b) const { return a.d < b.d; } };

static DynArray<int> tracePath(const HashMap<int,int>& prev, int dst) {
    DynArray<int> p;
    for (int cur=dst; cur!=-1; ) {
        p.push_back(cur);
        const int* pp = prev.find(cur);
        cur = pp ? *pp : -1;
    }
    for (int l=0, r=p.size()-1; l<r; ++l, --r) { int t=p[l]; p[l]=p[r]; p[r]=t; }
    return p;
}

static void initMaps(const DynArray<int>& ids,
                     HashMap<int,double>& da, HashMap<int,double>& db,
                     HashMap<int,int>& prev) {
    for (int i=0; i<ids.size(); ++i) { da[ids[i]]=INF; db[ids[i]]=INF; prev[ids[i]]=-1; }
}

HashMap<int, PathResult> Dijkstra::shortestTimeFrom(const Graph& g, int src) {
    if (!g.hasNode(src)) { LOG_ERROR("Dijkstra起点不存在"); return {}; }
    DynArray<int> ids = g.getAllNodeIds();
    HashMap<int,double> dist, cost; HashMap<int,int> prev;
    initMaps(ids, dist, cost, prev);
    dist[src] = cost[src] = 0;

    MinHeap<DE,DECmp> pq; pq.push({0.0, src});
    while (!pq.empty()) {
        DE top=pq.top(); pq.pop();
        double* dp=dist.find(top.n);
        if (!dp || top.d > *dp) continue;
        for (const Edge& e : g.getNeighbors(top.n)) {
            double nd = *dp + e.time;
            double* dv=dist.find(e.to);
            if (dv && nd < *dv) {
                *dv=nd; *cost.find(e.to) = *cost.find(top.n)+e.cost;
                prev[e.to]=top.n; pq.push({nd, e.to});
            }
        }
    }
    HashMap<int,PathResult> res;
    for (int i=0; i<ids.size(); ++i) {
        PathResult pr; double* dv=dist.find(ids[i]);
        if ((pr.reachable = (dv && *dv!=INF))) {
            pr.totalTime=*dv; pr.totalCost=*cost.find(ids[i]);
            pr.path=tracePath(prev, ids[i]);
        }
        res.set(ids[i], pr);
    }
    return res;
}

PathResult Dijkstra::cheapestPath(const Graph& g, int src, int dst) {
    if (!g.hasNode(src)||!g.hasNode(dst)) { LOG_ERROR("cheapestPath：节点不存在"); return {}; }
    DynArray<int> ids=g.getAllNodeIds();
    HashMap<int,double> dist, tm; HashMap<int,int> prev;
    initMaps(ids, dist, tm, prev);
    dist[src]=tm[src]=0;

    MinHeap<DE,DECmp> pq; pq.push({0.0, src});
    while (!pq.empty()) {
        DE top=pq.top(); pq.pop();
        double* dp=dist.find(top.n);
        if (!dp || top.d > *dp) continue;
        for (const Edge& e : g.getNeighbors(top.n)) {
            double nd = *dp + e.cost;
            double* dv=dist.find(e.to);
            if (dv && nd < *dv) {
                *dv=nd; *tm.find(e.to) = *tm.find(top.n)+e.time;
                prev[e.to]=top.n; pq.push({nd, e.to});
            }
        }
    }
    PathResult pr; double* dv=dist.find(dst);
    if ((pr.reachable = (dv && *dv!=INF))) {
        pr.totalCost=*dv; pr.totalTime=*tm.find(dst);
        pr.path=tracePath(prev, dst);
    }
    return pr;
}
