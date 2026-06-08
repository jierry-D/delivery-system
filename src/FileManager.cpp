#include "../include/FileManager.h"
#include "../include/Logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

bool FileManager::loadNetwork(const string& path, Graph& graph) {
    ifstream fin(path);
    if (!fin.is_open()) {
        LOG_ERROR("无法打开文件：" + path);
        return false;
    }

    graph.clear();
    string line, section;
    int nodeCount = 0, edgeCount = 0;

    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') {
            if (line.find("NODES") != string::npos) section = "NODES";
            else if (line.find("EDGES") != string::npos) section = "EDGES";
            continue;
        }

        istringstream iss(line);
        if (section == "NODES") {
            int id;
            string name, address;
            if (iss >> id >> name) {
                getline(iss, address);
                if (!address.empty() && address[0] == ' ')
                    address = address.substr(1);
                if (graph.addNode(Node(id, name, address))) nodeCount++;
            }
        } else if (section == "EDGES") {
            int from, to;
            double time, cost;
            if (iss >> from >> to >> time >> cost) {
                if (graph.addEdge(Edge(from, to, time, cost))) edgeCount++;
            }
        }
    }

    LOG_INFO("路网导入完成：" + to_string(nodeCount) + " 个网点，" +
             to_string(edgeCount) + " 条路段，来自 " + path);
    return true;
}

bool FileManager::saveNetwork(const string& path, const Graph& graph) {
    ofstream fout(path);
    if (!fout.is_open()) {
        LOG_ERROR("无法写入文件：" + path);
        return false;
    }

    fout << "# NODES\n";
    auto ids = graph.getAllNodeIds();
    sort(ids.begin(), ids.end());
    for (int id : ids) {
        const Node* n = graph.findNode(id);
        if (n) fout << n->id << " " << n->name << " " << n->address << "\n";
    }

    fout << "# EDGES\n";
    for (int from : ids) {
        for (const Edge& e : graph.getNeighbors(from)) {
            fout << fixed << setprecision(2)
                 << e.from << " " << e.to << " "
                 << e.time << " " << e.cost << "\n";
        }
    }

    LOG_INFO("路网已保存到：" + path);
    return true;
}

bool FileManager::loadOrders(const string& path, OrderManager& om) {
    ifstream fin(path);
    if (!fin.is_open()) {
        LOG_ERROR("无法打开订单文件：" + path);
        return false;
    }

    int count = 0;
    string line;
    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream iss(line);
        int orderId, src, dst, byTime;
        string goods;
        if (iss >> orderId >> src >> dst >> goods >> byTime) {
            Order order{orderId, src, dst, goods, byTime != 0};
            if (om.addOrder(order)) count++;
        }
    }

    LOG_INFO("批量导入订单完成：" + to_string(count) + " 条，来自 " + path);
    return true;
}

bool FileManager::savePlans(const string& path,
                             const Graph& /*graph*/,
                             const vector<DeliveryPlan>& plans)
{
    ofstream fout(path);
    if (!fout.is_open()) {
        LOG_ERROR("无法写入配送方案文件：" + path);
        return false;
    }

    fout << fixed << setprecision(2);
    fout << "# 配送方案输出\n";
    fout << "# 订单号 货物 优化目标 总耗时(h) 总费用(元) 路径\n";

    for (const auto& plan : plans) {
        const Order& o = plan.order;
        const PathResult& r = plan.result;
        fout << o.orderId << " " << o.goods << " "
             << (o.byTime ? "最短耗时" : "最低费用") << " ";
        if (!r.reachable) {
            fout << "不可达\n";
            continue;
        }
        fout << r.totalTime << " " << r.totalCost << " ";
        for (size_t i = 0; i < r.path.size(); ++i) {
            fout << r.path[i];
            if (i + 1 < r.path.size()) fout << "->";
        }
        fout << "\n";
    }

    LOG_INFO("配送方案已保存到：" + path);
    return true;
}
