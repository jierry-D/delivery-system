#include "../include/Graph.h"
#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

const std::vector<Edge> Graph::emptyEdges;

bool Graph::addNode(const Node& node) {
    if (node.name.empty()) {
        LOG_WARN("添加网点失败：名称不能为空");
        return false;
    }
    if (nodes.count(node.id)) {
        LOG_WARN("添加网点失败：编号 " + std::to_string(node.id) + " 已存在");
        return false;
    }
    nodes[node.id] = node;
    adjList[node.id]; // 初始化空邻接表
    LOG_INFO("添加网点成功：[" + std::to_string(node.id) + "] " + node.name);
    return true;
}

bool Graph::deleteNode(int id) {
    if (!nodes.count(id)) {
        LOG_WARN("删除失败：网点 " + std::to_string(id) + " 不存在");
        return false;
    }
    nodes.erase(id);
    adjList.erase(id);
    // 删除所有以该节点为终点的边
    for (auto& [from, edges] : adjList) {
        edges.erase(
            std::remove_if(edges.begin(), edges.end(),
                           [id](const Edge& e) { return e.to == id; }),
            edges.end());
    }
    LOG_INFO("删除网点成功：" + std::to_string(id));
    return true;
}

bool Graph::updateNode(int id, const Node& node) {
    if (!nodes.count(id)) {
        LOG_WARN("修改失败：网点 " + std::to_string(id) + " 不存在");
        return false;
    }
    if (node.name.empty()) {
        LOG_WARN("修改失败：名称不能为空");
        return false;
    }
    nodes[id] = node;
    nodes[id].id = id;
    LOG_INFO("修改网点成功：" + std::to_string(id));
    return true;
}

Node* Graph::findNode(int id) {
    auto it = nodes.find(id);
    return it != nodes.end() ? &it->second : nullptr;
}

const Node* Graph::findNode(int id) const {
    auto it = nodes.find(id);
    return it != nodes.end() ? &it->second : nullptr;
}

void Graph::listAllNodes() const {
    if (nodes.empty()) {
        std::cout << "  （当前无网点）\n";
        return;
    }
    std::cout << std::left
              << std::setw(6)  << "编号"
              << std::setw(16) << "名称"
              << "地址\n";
    std::cout << std::string(50, '-') << "\n";

    // 按 id 排序输出
    std::vector<int> ids;
    for (auto& [id, _] : nodes) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    for (int id : ids) {
        const Node& n = nodes.at(id);
        std::cout << std::setw(6)  << n.id
                  << std::setw(16) << n.name
                  << n.address << "\n";
    }
}

bool Graph::addEdge(const Edge& edge) {
    if (!nodes.count(edge.from) || !nodes.count(edge.to)) {
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
    // 检查是否已存在
    for (const auto& e : adjList[edge.from]) {
        if (e.to == edge.to) {
            LOG_WARN("添加路段失败：路段已存在");
            return false;
        }
    }
    adjList[edge.from].push_back(edge);
    LOG_INFO("添加路段：" + std::to_string(edge.from) + " -> " +
             std::to_string(edge.to) +
             " 耗时=" + std::to_string(edge.time) +
             "h 费用=" + std::to_string(edge.cost) + "元");
    return true;
}

bool Graph::deleteEdge(int from, int to) {
    auto it = adjList.find(from);
    if (it == adjList.end()) {
        LOG_WARN("删除路段失败：起点 " + std::to_string(from) + " 不存在");
        return false;
    }
    auto& edges = it->second;
    auto prev_size = edges.size();
    edges.erase(
        std::remove_if(edges.begin(), edges.end(),
                       [to](const Edge& e) { return e.to == to; }),
        edges.end());
    if (edges.size() == prev_size) {
        LOG_WARN("删除路段失败：路段不存在");
        return false;
    }
    LOG_INFO("删除路段：" + std::to_string(from) + " -> " + std::to_string(to));
    return true;
}

void Graph::listAllEdges() const {
    std::cout << std::left
              << std::setw(8)  << "起点"
              << std::setw(8)  << "终点"
              << std::setw(12) << "耗时(h)"
              << "费用(元)\n";
    std::cout << std::string(45, '-') << "\n";

    std::vector<int> ids;
    for (auto& [id, _] : adjList) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    for (int from : ids) {
        for (const Edge& e : adjList.at(from)) {
            std::cout << std::setw(8)  << e.from
                      << std::setw(8)  << e.to
                      << std::setw(12) << e.time
                      << e.cost << "\n";
        }
    }
}

const std::vector<Edge>& Graph::getNeighbors(int id) const {
    auto it = adjList.find(id);
    return it != adjList.end() ? it->second : emptyEdges;
}

std::vector<int> Graph::getAllNodeIds() const {
    std::vector<int> ids;
    for (auto& [id, _] : nodes) ids.push_back(id);
    return ids;
}

bool Graph::hasNode(int id) const {
    return nodes.count(id) > 0;
}

int Graph::nodeCount() const {
    return static_cast<int>(nodes.size());
}

int Graph::edgeCount() const {
    int cnt = 0;
    for (auto& [_, edges] : adjList) cnt += edges.size();
    return cnt;
}

void Graph::clear() {
    nodes.clear();
    adjList.clear();
}
