#include "../include/OrderManager.h"
#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <string>

using std::to_string;

bool OrderManager::orderExists(int orderId) const {
    for (int i = 0; i < orders.size(); ++i)
        if (orders[i].orderId == orderId) return true;
    return false;
}

bool OrderManager::addOrder(const Order& order) {
    if (orderExists(order.orderId)) {
        LOG_WARN("订单 " + to_string(order.orderId) + " 已存在");
        return false;
    }
    orders.push_back(order);
    LOG_INFO("添加订单 " + to_string(order.orderId) + "：" +
             to_string(order.srcNode) + " -> " + to_string(order.dstNode));
    return true;
}

bool OrderManager::removeOrder(int orderId) {
    bool ok = orders.remove_first_if(
        [orderId](const Order& o) { return o.orderId == orderId; });
    if (!ok) { LOG_WARN("订单 " + to_string(orderId) + " 不存在"); return false; }
    LOG_INFO("删除订单 " + to_string(orderId));
    return true;
}

void OrderManager::listOrders() const {
    if (orders.empty()) {
        std::cout << "  （当前无订单）\n";
        return;
    }
    std::cout << std::left
              << std::setw(8)  << "订单号"
              << std::setw(8)  << "起点"
              << std::setw(8)  << "终点"
              << std::setw(16) << "货物"
              << "优化目标\n"
              << std::string(50, '-') << "\n";
    for (int i = 0; i < orders.size(); ++i) {
        const Order& o = orders[i];
        std::cout << std::setw(8)  << o.orderId
                  << std::setw(8)  << o.srcNode
                  << std::setw(8)  << o.dstNode
                  << std::setw(16) << o.goods
                  << (o.byTime ? "最短耗时" : "最低费用") << "\n";
    }
}

DynArray<DeliveryPlan> OrderManager::planAllOrders(const Graph& g) const {
    DynArray<DeliveryPlan> plans;
    for (int i = 0; i < orders.size(); ++i) {
        const Order& order = orders[i];
        DeliveryPlan plan;
        plan.order = order;
        if (order.byTime) {
            HashMap<int, PathResult> all = Dijkstra::shortestTimeFrom(g, order.srcNode);
            PathResult* pr = all.find(order.dstNode);
            plan.result = pr ? *pr : PathResult{};
        } else {
            plan.result = Dijkstra::cheapestPath(g, order.srcNode, order.dstNode);
        }
        plans.push_back(plan);
    }
    LOG_INFO("完成批量路径规划，共 " + to_string(plans.size()) + " 条订单");
    return plans;
}

TopoResult OrderManager::planBatchSequence(const Graph& g) const {
    // 收集所有订单涉及节点（去重）
    HashMap<int, bool> seen;
    DynArray<int> nodeIds;
    for (int i = 0; i < orders.size(); ++i) {
        if (!seen.contains(orders[i].srcNode)) {
            seen.set(orders[i].srcNode, true);
            nodeIds.push_back(orders[i].srcNode);
        }
        if (!seen.contains(orders[i].dstNode)) {
            seen.set(orders[i].dstNode, true);
            nodeIds.push_back(orders[i].dstNode);
        }
    }
    return TopoSort::sort(g, nodeIds);
}

void OrderManager::printPlan(const Graph& g, const DeliveryPlan& plan) const {
    const Order&      o = plan.order;
    const PathResult& r = plan.result;

    std::cout << "\n订单 " << o.orderId << "  货物：" << o.goods
              << "  优化目标：" << (o.byTime ? "最短耗时" : "最低费用") << "\n";
    if (!r.reachable) {
        std::cout << "  [不可达]\n";
        return;
    }
    std::cout << "  路径：";
    for (int i = 0; i < r.path.size(); ++i) {
        int id = r.path[i];
        const Node* n = g.findNode(id);
        std::cout << id << "(" << (n ? n->name : "?") << ")";
        if (i + 1 < r.path.size()) std::cout << " -> ";
    }
    std::cout << std::fixed << std::setprecision(2)
              << "\n  总耗时：" << r.totalTime << " 小时"
              << "  总费用：" << r.totalCost << " 元\n";
}
