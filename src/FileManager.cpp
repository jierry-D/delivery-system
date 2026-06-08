#include "../include/FileManager.h"
#include "../include/Logger.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

using std::string;
using std::to_string;

bool FileManager::loadNetwork(const string& path, Graph& graph) {
    std::ifstream fin(path);
    if (!fin.is_open()) { LOG_ERROR("无法打开文件：" + path); return false; }

    graph.clear();
    string line, section;
    int nc = 0, ec = 0;

    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') {
            if (line.find("NODES") != string::npos) section = "NODES";
            else if (line.find("EDGES") != string::npos) section = "EDGES";
            continue;
        }
        std::istringstream iss(line);
        if (section == "NODES") {
            int id;
            string name, addr;
            if (iss >> id >> name) {
                std::getline(iss, addr);
                if (!addr.empty() && addr[0] == ' ') addr = addr.substr(1);
                if (graph.addNode(Node(id, name, addr))) ++nc;
            }
        } else if (section == "EDGES") {
            int from, to;
            double t, c;
            if (iss >> from >> to >> t >> c)
                if (graph.addEdge(Edge(from, to, t, c))) ++ec;
        }
    }
    LOG_INFO("路网导入完成：" + to_string(nc) + " 网点 " + to_string(ec) + " 路段，来自 " + path);
    return true;
}

bool FileManager::saveNetwork(const string& path, const Graph& graph) {
    std::ofstream fout(path);
    if (!fout.is_open()) { LOG_ERROR("无法写入文件：" + path); return false; }

    DynArray<int> ids = graph.getAllNodeIds();
    // 排序
    for (int i = 0; i < ids.size() - 1; ++i)
        for (int j = i + 1; j < ids.size(); ++j)
            if (ids[i] > ids[j]) { int t = ids[i]; ids[i] = ids[j]; ids[j] = t; }

    fout << "# NODES\n";
    for (int i = 0; i < ids.size(); ++i) {
        const Node* n = graph.findNode(ids[i]);
        if (n) fout << n->id << " " << n->name << " " << n->address << "\n";
    }
    fout << "# EDGES\n";
    for (int i = 0; i < ids.size(); ++i) {
        const DynArray<Edge>& es = graph.getNeighbors(ids[i]);
        for (int j = 0; j < es.size(); ++j) {
            fout << std::fixed << std::setprecision(2)
                 << es[j].from << " " << es[j].to << " "
                 << es[j].time << " " << es[j].cost << "\n";
        }
    }
    LOG_INFO("路网已保存到：" + path);
    return true;
}

bool FileManager::loadOrders(const string& path, OrderManager& om) {
    std::ifstream fin(path);
    if (!fin.is_open()) { LOG_ERROR("无法打开订单文件：" + path); return false; }

    int count = 0;
    string line;
    while (std::getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        int oid, src, dst, bt;
        string goods;
        if (iss >> oid >> src >> dst >> goods >> bt) {
            Order o{oid, src, dst, goods, bt != 0};
            if (om.addOrder(o)) ++count;
        }
    }
    LOG_INFO("批量导入订单完成：" + to_string(count) + " 条，来自 " + path);
    return true;
}

bool FileManager::savePlans(const string& path,
                             const Graph& /*graph*/,
                             const DynArray<DeliveryPlan>& plans)
{
    std::ofstream fout(path);
    if (!fout.is_open()) { LOG_ERROR("无法写入配送方案文件：" + path); return false; }

    fout << std::fixed << std::setprecision(2);
    fout << "# 配送方案输出\n";
    for (int i = 0; i < plans.size(); ++i) {
        const Order&      o = plans[i].order;
        const PathResult& r = plans[i].result;
        fout << o.orderId << " " << o.goods << " "
             << (o.byTime ? "最短耗时" : "最低费用") << " ";
        if (!r.reachable) { fout << "不可达\n"; continue; }
        fout << r.totalTime << " " << r.totalCost << " ";
        for (int j = 0; j < r.path.size(); ++j) {
            fout << r.path[j];
            if (j + 1 < r.path.size()) fout << "->";
        }
        fout << "\n";
    }
    LOG_INFO("配送方案已保存到：" + path);
    return true;
}
