#pragma once
#include "Graph.h"
#include "Dijkstra.h"
#include "TopoSort.h"
#include "DynArray.h"
#include <string>

struct Order {
    int         orderId;
    int         srcNode;
    int         dstNode;
    std::string goods;
    bool        byTime;  // true=按耗时最短, false=按费用最低
};

struct DeliveryPlan {
    Order      order;
    PathResult result;
};

class OrderManager {
public:
    bool addOrder(const Order& order);
    bool removeOrder(int orderId);
    void listOrders() const;

    DynArray<DeliveryPlan> planAllOrders(const Graph& g) const;
    TopoResult             planBatchSequence(const Graph& g) const;
    void printPlan(const Graph& g, const DeliveryPlan& plan) const;

    const DynArray<Order>& getOrders() const { return orders; }
    void clear() { orders.clear(); }

private:
    DynArray<Order> orders;
    bool orderExists(int orderId) const;
};
