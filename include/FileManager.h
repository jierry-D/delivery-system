#pragma once
#include "Graph.h"
#include "OrderManager.h"
#include <string>

class FileManager {
public:
    static bool loadNetwork(const std::string& path, Graph& graph);
    static bool saveNetwork(const std::string& path, const Graph& graph);
    static bool loadOrders(const std::string& path, OrderManager& om);
    static bool savePlans(const std::string& path,
                          const Graph& graph,
                          const DynArray<DeliveryPlan>& plans);
};
