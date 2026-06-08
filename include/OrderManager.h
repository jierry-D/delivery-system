#pragma once
#include "Graph.h"
#include "Dijkstra.h"
#include "TopoSort.h"
#include <vector>
#include <string>

struct Order {
    int orderId;
    int srcNode;
    int dstNode;
    std::string goods;
    bool byTime;  // true=按耗时最短, false=按费用最低
};

struct DeliveryPlan {
    Order order;
    PathResult result;
};

class OrderManager {
public:
    bool addOrder(const Order& order);
    void listOrders() const;
    bool removeOrder(int orderId);

    // 批量规划所有订单
    std::vector<DeliveryPlan> planAllOrders(const Graph& g) const;

    // 批次配送拓扑排序：取所有订单涉及节点
    TopoResult planBatchSequence(const Graph& g) const;

    void printPlan(const Graph& g, const DeliveryPlan& plan) const;

    const std::vector<Order>& getOrders() const { return orders; }
    void clear() { orders.clear(); }

private:
    std::vector<Order> orders;
    bool orderExists(int orderId) const;
};
