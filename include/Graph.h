#pragma once
#include "Containers.h"
#include <string>

// ── 基本数据结构 ──────────────────────────────────────────
struct Node {
    int id = 0;
    std::string name, address;
    Node() = default;
    Node(int i, const std::string& n, const std::string& a) : id(i), name(n), address(a) {}
};

struct Edge {
    int    from=0, to=0;
    double time=0, cost=0;
    Edge() = default;
    Edge(int f, int t, double ti, double co) : from(f), to(t), time(ti), cost(co) {}
};

// ── 有向加权图（邻接表） ──────────────────────────────────
class Graph {
public:
    bool  addNode(const Node& n);
    bool  deleteNode(int id);
    bool  updateNode(int id, const Node& n);
    Node*       findNode(int id);
    const Node* findNode(int id) const;
    void  listAllNodes() const;

    bool  addEdge(const Edge& e);
    bool  deleteEdge(int from, int to);
    void  listAllEdges() const;

    const DynArray<Edge>& getNeighbors(int id) const;
    DynArray<int>         getAllNodeIds() const;
    bool hasNode(int id) const;
    int  nodeCount()     const;
    int  edgeCount()     const;
    void clear();

private:
    HashMap<int, Node>           nodes;
    HashMap<int, DynArray<Edge>> adj;
    static const DynArray<Edge>  empty_;
};
