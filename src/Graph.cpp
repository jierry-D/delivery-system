#include "../include/Graph.h"
#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <string>

using std::string;
using std::to_string;

const DynArray<Edge> Graph::emptyEdges;

bool Graph::addNode(const Node& node) {
    if (node.name.empty()) {
        LOG_WARN("添加网点失败：名称不能为空");
        return false;
    }
    if (nodes.contains(node.id)) {
        LOG_WARN("添加网点失败：编号 " + to_string(node.id) + " 已存在");
        return false;
    }
    nodes.set(node.id, node);
    adjList[node.id];  // 初始化空邻接表
    LOG_INFO("添加网点成功：[" + to_string(node.id) + "] " + node.name);
    return true;
}

bool Graph::deleteNode(int id) {
    if (!nodes.contains(id)) {
        LOG_WARN("删除失败：网点 " + to_string(id) + " 不存在");
        return false;
    }
    nodes.erase(id);
    adjList.erase(id);
    // 删除所有以该节点为终点的边
    adjList.forEach([id](int, DynArray<Edge>& edges) {
        edges.remove_all_if([id](const Edge& e) { return e.to == id; });
    });
    LOG_INFO("删除网点成功：" + to_string(id));
    return true;
}

bool Graph::updateNode(int id, const Node& node) {
    if (!nodes.contains(id)) {
        LOG_WARN("修改失败：网点 " + to_string(id) + " 不存在");
        return false;
    }
    if (node.name.empty()) {
        LOG_WARN("修改失败：名称不能为空");
        return false;
    }
    Node updated = node;
    updated.id = id;
    nodes.set(id, updated);
    LOG_INFO("修改网点成功：" + to_string(id));
    return true;
}

Node* Graph::findNode(int id) {
    return nodes.find(id);
}

const Node* Graph::findNode(int id) const {
    return nodes.find(id);
}

void Graph::listAllNodes() const {
    if (nodes.empty()) {
        std::cout << "  （当前无网点）\n";
        return;
    }
    // 收集并排序 id
    DynArray<int> ids = getAllNodeIds();
    for (int i = 0; i < ids.size() - 1; ++i)
        for (int j = i + 1; j < ids.size(); ++j)
            if (ids[i] > ids[j]) {
                int tmp = ids[i]; ids[i] = ids[j]; ids[j] = tmp;
            }

    std::cout << std::left
              << std::setw(6)  << "编号"
              << std::setw(16) << "名称"
              << "地址\n"
              << std::string(50, '-') << "\n";
    for (int i = 0; i < ids.size(); ++i) {
        const Node* n = nodes.find(ids[i]);
        if (n)
            std::cout << std::setw(6)  << n->id
                      << std::setw(16) << n->name
                      << n->address << "\n";
    }
}

bool Graph::addEdge(const Edge& edge) {
    if (!nodes.contains(edge.from) || !nodes.contains(edge.to)) {
        LOG_WARN("添加路段失败：起点或终点不存在");
        return false;
    }
    if (edge.from == edge.to) {
        LOG_WARN("添加路段失败：不允许自环");
        return false;
    }
    if (edge.time < 0 || edge.cost < 0) {
        LOG_WARN("添加路段失败：权重不能为负");
        return false;
    }
    DynArray<Edge>& edges = adjList[edge.from];
    for (int i = 0; i < edges.size(); ++i) {
        if (edges[i].to == edge.to) {
            LOG_WARN("添加路段失败：路段已存在");
            return false;
        }
    }
    edges.push_back(edge);
    LOG_INFO("添加路段：" + to_string(edge.from) + " -> " + to_string(edge.to) +
             " 耗时=" + to_string(edge.time) + "h 费用=" + to_string(edge.cost) + "元");
    return true;
}

bool Graph::deleteEdge(int from, int to) {
    DynArray<Edge>* ep = adjList.find(from);
    if (!ep) {
        LOG_WARN("删除路段失败：起点 " + to_string(from) + " 不存在");
        return false;
    }
    bool ok = ep->remove_first_if([to](const Edge& e) { return e.to == to; });
    if (!ok) { LOG_WARN("删除路段失败：路段不存在"); return false; }
    LOG_INFO("删除路段：" + to_string(from) + " -> " + to_string(to));
    return true;
}

void Graph::listAllEdges() const {
    DynArray<int> ids = getAllNodeIds();
    for (int i = 0; i < ids.size() - 1; ++i)
        for (int j = i + 1; j < ids.size(); ++j)
            if (ids[i] > ids[j]) { int t = ids[i]; ids[i] = ids[j]; ids[j] = t; }

    std::cout << std::left
              << std::setw(8)  << "起点"
              << std::setw(8)  << "终点"
              << std::setw(12) << "耗时(h)"
              << "费用(元)\n"
              << std::string(45, '-') << "\n";
    for (int i = 0; i < ids.size(); ++i) {
        const DynArray<Edge>* ep = adjList.find(ids[i]);
        if (!ep) continue;
        for (int j = 0; j < ep->size(); ++j) {
            const Edge& e = (*ep)[j];
            std::cout << std::setw(8)  << e.from
                      << std::setw(8)  << e.to
                      << std::setw(12) << e.time
                      << e.cost << "\n";
        }
    }
}

const DynArray<Edge>& Graph::getNeighbors(int id) const {
    const DynArray<Edge>* ep = adjList.find(id);
    return ep ? *ep : emptyEdges;
}

DynArray<int> Graph::getAllNodeIds() const {
    DynArray<int> ids;
    nodes.forEach([&](int id, const Node&) { ids.push_back(id); });
    return ids;
}

bool Graph::hasNode(int id) const {
    return nodes.contains(id);
}

int Graph::nodeCount() const {
    return nodes.size();
}

int Graph::edgeCount() const {
    int cnt = 0;
    adjList.forEach([&](int, const DynArray<Edge>& e) { cnt += e.size(); });
    return cnt;
}

void Graph::clear() {
    nodes.clear();
    adjList.clear();
}
