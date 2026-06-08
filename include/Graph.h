#pragma once
#include "Node.h"
#include "Edge.h"
#include "DynArray.h"
#include "HashMap.h"
#include <string>

class Graph {
public:
    bool  addNode(const Node& node);
    bool  deleteNode(int id);
    bool  updateNode(int id, const Node& node);
    Node*       findNode(int id);
    const Node* findNode(int id) const;
    void  listAllNodes() const;

    bool  addEdge(const Edge& edge);
    bool  deleteEdge(int from, int to);
    void  listAllEdges() const;

    const DynArray<Edge>& getNeighbors(int id) const;
    DynArray<int>         getAllNodeIds() const;
    bool  hasNode(int id) const;
    int   nodeCount() const;
    int   edgeCount() const;
    void  clear();

private:
    HashMap<int, Node>           nodes;
    HashMap<int, DynArray<Edge>> adjList;
    static const DynArray<Edge>  emptyEdges;
};
