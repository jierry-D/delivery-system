#pragma once
#include "Node.h"
#include "Edge.h"
#include <unordered_map>
#include <vector>
#include <string>

class Graph {
public:
    bool addNode(const Node& node);
    bool deleteNode(int id);
    bool updateNode(int id, const Node& node);
    Node* findNode(int id);
    const Node* findNode(int id) const;
    void listAllNodes() const;

    bool addEdge(const Edge& edge);
    bool deleteEdge(int from, int to);
    void listAllEdges() const;

    const std::vector<Edge>& getNeighbors(int id) const;
    std::vector<int> getAllNodeIds() const;
    bool hasNode(int id) const;
    int nodeCount() const;
    int edgeCount() const;
    void clear();

private:
    std::unordered_map<int, Node> nodes;
    std::unordered_map<int, std::vector<Edge>> adjList;
    static const std::vector<Edge> emptyEdges;
};
