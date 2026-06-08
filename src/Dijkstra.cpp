#include "../include/Dijkstra.h"
#include "../include/Logger.h"
#include <queue>
#include <limits>
#include <algorithm>

using namespace std;
static const double INF = numeric_limits<double>::infinity();

map<int, PathResult> Dijkstra::shortestTimeFrom(const Graph& g, int src) {
    if (!g.hasNode(src)) {
        LOG_ERROR("Dijkstra 起点 " + to_string(src) + " 不存在");
        return {};
    }

    auto ids = g.getAllNodeIds();
    map<int, double> dist, costDist;
    map<int, int> prev;
    for (int id : ids) {
        dist[id] = INF; costDist[id] = INF; prev[id] = -1;
    }
    dist[src] = 0; costDist[src] = 0;

    // 小顶堆 {distance, nodeId}
    priority_queue<pair<double,int>,
                   vector<pair<double,int>>,
                   greater<pair<double,int>>> pq;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (const Edge& e : g.getNeighbors(u)) {
            int v = e.to;
            if (!g.hasNode(v)) continue;
            double nd = dist[u] + e.time;
            if (nd < dist[v]) {
                dist[v] = nd;
                costDist[v] = costDist[u] + e.cost;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }

    map<int, PathResult> results;
    for (int id : ids) {
        PathResult pr;
        pr.reachable = (dist[id] != INF);
        if (pr.reachable) {
            pr.totalTime = dist[id];
            pr.totalCost = costDist[id];
            vector<int> path;
            for (int cur = id; cur != -1; cur = prev[cur])
                path.push_back(cur);
            reverse(path.begin(), path.end());
            pr.path = path;
        }
        results[id] = pr;
    }
    return results;
}

PathResult Dijkstra::cheapestPath(const Graph& g, int src, int dst) {
    if (!g.hasNode(src) || !g.hasNode(dst)) {
        LOG_ERROR("cheapestPath：起点或终点不存在");
        return PathResult{};
    }

    auto ids = g.getAllNodeIds();
    map<int, double> dist, timeDist;
    map<int, int> prev;
    for (int id : ids) {
        dist[id] = INF; timeDist[id] = INF; prev[id] = -1;
    }
    dist[src] = 0; timeDist[src] = 0;

    priority_queue<pair<double,int>,
                   vector<pair<double,int>>,
                   greater<pair<double,int>>> pq;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (const Edge& e : g.getNeighbors(u)) {
            int v = e.to;
            if (!g.hasNode(v)) continue;
            double nd = dist[u] + e.cost;
            if (nd < dist[v]) {
                dist[v] = nd;
                timeDist[v] = timeDist[u] + e.time;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }

    PathResult pr;
    pr.reachable = (dist[dst] != INF);
    if (pr.reachable) {
        pr.totalCost = dist[dst];
        pr.totalTime = timeDist[dst];
        vector<int> path;
        for (int cur = dst; cur != -1; cur = prev[cur])
            path.push_back(cur);
        reverse(path.begin(), path.end());
        pr.path = path;
    }
    return pr;
}
