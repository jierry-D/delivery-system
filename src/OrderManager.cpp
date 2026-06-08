#include "../include/OrderManager.h"
#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <algorithm>

using namespace std;

bool OrderManager::orderExists(int orderId) const {
    for (const auto& o : orders)
        if (o.orderId == orderId) return true;
    return false;
}

bool OrderManager::addOrder(const Order& order) {
    if (orderExists(order.orderId)) {
        LOG_WARN("订单 " + to_string(order.orderId) + " 已存在");
        return false;
    }
    orders.push_back(order);
    LOG_INFO("添加订单 " + to_string(order.orderId) +
             "：" + to_string(order.srcNode) + " -> " + to_string(order.dstNode));
    return true;
}

bool OrderManager::removeOrder(int orderId) {
    auto it = remove_if(orders.begin(), orders.end(),
                        [orderId](const Order& o){ return o.orderId == orderId; });
    if (it == orders.end()) {
        LOG_WARN("订单 " + to_string(orderId) + " 不存在");
        return false;
    }
    orders.erase(it, orders.end());
    LOG_INFO("删除订单 " + to_string(orderId));
    return true;
}

void OrderManager::listOrders() const {
    if (orders.empty()) {
        cout << "  （当前无订单）\n";
        return;
    }
    cout << left
         << setw(8)  << "订单号"
         << setw(8)  << "起点"
         << setw(8)  << "终点"
         << setw(12) << "货物"
         << "优化目标\n";
    cout << string(50, '-') << "\n";
    for (const auto& o : orders) {
        cout << setw(8)  << o.orderId
             << setw(8)  << o.srcNode
             << setw(8)  << o.dstNode
             << setw(12) << o.goods
             << (o.byTime ? "最短耗时" : "最低费用") << "\n";
    }
}

vector<DeliveryPlan> OrderManager::planAllOrders(const Graph& g) const {
    vector<DeliveryPlan> plans;
    for (const auto& order : orders) {
        DeliveryPlan plan;
        plan.order = order;
        if (order.byTime) {
            auto allPaths = Dijkstra::shortestTimeFrom(g, order.srcNode);
            auto it = allPaths.find(order.dstNode);
            plan.result = (it != allPaths.end()) ? it->second : PathResult{};
        } else {
            plan.result = Dijkstra::cheapestPath(g, order.srcNode, order.dstNode);
        }
        plans.push_back(plan);
    }
    LOG_INFO("完成批量路径规划，共 " + to_string(plans.size()) + " 条订单");
    return plans;
}

TopoResult OrderManager::planBatchSequence(const Graph& g) const {
    // 收集所有订单涉及的节点（去重）
    unordered_set<int> nodeSet;
    for (const auto& o : orders) {
        nodeSet.insert(o.srcNode);
        nodeSet.insert(o.dstNode);
    }
    vector<int> nodeIds(nodeSet.begin(), nodeSet.end());
    return TopoSort::sort(g, nodeIds);
}

void OrderManager::printPlan(const Graph& g, const DeliveryPlan& plan) const {
    const Order& o = plan.order;
    const PathResult& r = plan.result;

    cout << "\n订单 " << o.orderId << "  货物：" << o.goods
         << "  优化目标：" << (o.byTime ? "最短耗时" : "最低费用") << "\n";

    if (!r.reachable) {
        cout << "  [不可达] 起点 " << o.srcNode
             << " 到终点 " << o.dstNode << " 无路径\n";
        return;
    }

    cout << "  路径：";
    for (size_t i = 0; i < r.path.size(); ++i) {
        int id = r.path[i];
        const Node* node = g.findNode(id);
        cout << id << "(" << (node ? node->name : "?") << ")";
        if (i + 1 < r.path.size()) cout << " -> ";
    }
    cout << "\n";
    cout << fixed << setprecision(2);
    cout << "  总耗时：" << r.totalTime << " 小时"
         << "  总费用：" << r.totalCost << " 元\n";
}
