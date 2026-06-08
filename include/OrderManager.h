#pragma once
#include "Dijkstra.h"
#include "TopoSort.h"
#include <string>

// ── 订单与配送计划 ────────────────────────────────────────
struct Order {
    int orderId, srcNode, dstNode;
    std::string goods;
    bool byTime;  // true=按耗时, false=按费用
};

struct DeliveryPlan { Order order; PathResult result; };

// ── 订单管理器 ────────────────────────────────────────────
class OrderManager {
public:
    bool addOrder(const Order& o);
    bool removeOrder(int id);
    void listOrders() const;
    DynArray<DeliveryPlan> planAllOrders(const Graph& g) const;
    TopoResult             planBatchSequence(const Graph& g) const;
    const DynArray<Order>& getOrders() const { return orders_; }
    void clear() { orders_.clear(); }
private:
    DynArray<Order> orders_;
};

// ── 文件管理器（静态工具类） ──────────────────────────────
class FileManager {
public:
    static bool loadNetwork(const std::string& path, Graph& g);
    static bool saveNetwork(const std::string& path, const Graph& g);
    static bool loadOrders (const std::string& path, OrderManager& om);
    static bool savePlans  (const std::string& path, const Graph& g,
                            const DynArray<DeliveryPlan>& plans);
};
