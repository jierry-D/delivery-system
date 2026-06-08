#include "include/Graph.h"
#include "include/Dijkstra.h"
#include "include/TopoSort.h"
#include "include/OrderManager.h"
#include "include/FileManager.h"
#include "include/Logger.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

static Graph g;
static OrderManager om;

// ─── 工具函数 ────────────────────────────────────────────
static void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static int readInt(const string& prompt) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v) { clearInput(); return v; }
        cout << "  输入无效，请输入整数\n";
        clearInput();
    }
}

static double readDouble(const string& prompt) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= 0) { clearInput(); return v; }
        cout << "  输入无效，请输入非负数\n";
        clearInput();
    }
}

static string readLine(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    return s;
}

static void pause() {
    cout << "\n按 Enter 继续...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ─── 打印路径结果 ──────────────────────────────────────
static void printPathResult(const PathResult& r, const string& label = "") {
    if (!label.empty()) cout << label << "\n";
    if (!r.reachable) {
        cout << "  [不可达]\n";
        return;
    }
    cout << "  路径：";
    for (size_t i = 0; i < r.path.size(); ++i) {
        int id = r.path[i];
        const Node* n = g.findNode(id);
        cout << id << "(" << (n ? n->name : "?") << ")";
        if (i + 1 < r.path.size()) cout << " -> ";
    }
    cout << fixed << setprecision(2)
         << "\n  总耗时：" << r.totalTime << " 小时"
         << "  总费用：" << r.totalCost << " 元\n";
}

// ─── 菜单 1：网点管理 ─────────────────────────────────
static void menuNode() {
    while (true) {
        cout << "\n--- 网点管理 ---\n"
             << "  1. 添加网点\n"
             << "  2. 删除网点\n"
             << "  3. 修改网点信息\n"
             << "  4. 查询网点\n"
             << "  5. 显示所有网点\n"
             << "  0. 返回\n";
        int ch = readInt("请选择：");
        if (ch == 0) break;

        if (ch == 1) {
            int id = readInt("  网点编号：");
            string name = readLine("  网点名称：");
            string addr = readLine("  网点地址：");
            g.addNode(Node(id, name, addr));

        } else if (ch == 2) {
            int id = readInt("  要删除的网点编号：");
            g.deleteNode(id);

        } else if (ch == 3) {
            int id = readInt("  要修改的网点编号：");
            string name = readLine("  新名称：");
            string addr = readLine("  新地址：");
            g.updateNode(id, Node(id, name, addr));

        } else if (ch == 4) {
            int id = readInt("  查询网点编号：");
            const Node* n = g.findNode(id);
            if (n)
                cout << "  编号：" << n->id
                     << "  名称：" << n->name
                     << "  地址：" << n->address << "\n";
            else
                cout << "  网点不存在\n";

        } else if (ch == 5) {
            cout << "\n当前共 " << g.nodeCount() << " 个网点：\n";
            g.listAllNodes();
        }
        pause();
    }
}

// ─── 菜单 2：路网管理 ─────────────────────────────────
static void menuNetwork() {
    while (true) {
        cout << "\n--- 路网管理 ---\n"
             << "  1. 添加路段\n"
             << "  2. 删除路段\n"
             << "  3. 显示所有路段\n"
             << "  4. 从文件导入路网\n"
             << "  5. 导出路网到文件\n"
             << "  0. 返回\n";
        int ch = readInt("请选择：");
        if (ch == 0) break;

        if (ch == 1) {
            int from = readInt("  起点编号：");
            int to   = readInt("  终点编号：");
            double t = readDouble("  耗时（小时）：");
            double c = readDouble("  费用（元）：");
            g.addEdge(Edge(from, to, t, c));

        } else if (ch == 2) {
            int from = readInt("  起点编号：");
            int to   = readInt("  终点编号：");
            g.deleteEdge(from, to);

        } else if (ch == 3) {
            cout << "\n当前共 " << g.edgeCount() << " 条路段：\n";
            g.listAllEdges();

        } else if (ch == 4) {
            string path = readLine("  文件路径（回车默认 data/network.txt）：");
            if (path.empty()) path = "data/network.txt";
            FileManager::loadNetwork(path, g);

        } else if (ch == 5) {
            string path = readLine("  保存路径（回车默认 data/network.txt）：");
            if (path.empty()) path = "data/network.txt";
            FileManager::saveNetwork(path, g);
        }
        pause();
    }
}

// ─── 菜单 3：路径查询 ─────────────────────────────────
static void menuPath() {
    while (true) {
        cout << "\n--- 路径查询 ---\n"
             << "  1. 单源最短耗时路径（Dijkstra）\n"
             << "  2. 两点最低费用路径\n"
             << "  0. 返回\n";
        int ch = readInt("请选择：");
        if (ch == 0) break;

        if (ch == 1) {
            int src = readInt("  起点网点编号：");
            auto results = Dijkstra::shortestTimeFrom(g, src);
            if (results.empty()) { pause(); continue; }

            cout << "\n从网点 " << src << " 出发的最短耗时路径：\n";
            auto ids = g.getAllNodeIds();
            sort(ids.begin(), ids.end());
            for (int id : ids) {
                if (id == src) continue;
                const Node* n = g.findNode(id);
                cout << "  -> " << id << "(" << (n ? n->name : "?") << ") ";
                auto& r = results[id];
                printPathResult(r);
            }

        } else if (ch == 2) {
            int src = readInt("  起点网点编号：");
            int dst = readInt("  终点网点编号：");
            auto r = Dijkstra::cheapestPath(g, src, dst);
            printPathResult(r, "最低费用路径：");
        }
        pause();
    }
}

// ─── 菜单 4：批次配送 ─────────────────────────────────
static void menuDelivery() {
    while (true) {
        cout << "\n--- 批次配送 ---\n"
             << "  1. 添加配送订单\n"
             << "  2. 删除订单\n"
             << "  3. 显示所有订单\n"
             << "  4. 批量导入订单（文件）\n"
             << "  5. 规划所有订单路径\n"
             << "  6. 批次配送顺序（拓扑排序）\n"
             << "  7. 导出配送方案\n"
             << "  0. 返回\n";
        int ch = readInt("请选择：");
        if (ch == 0) break;

        if (ch == 1) {
            int oid = readInt("  订单号：");
            int src = readInt("  起点网点编号：");
            int dst = readInt("  终点网点编号：");
            string goods = readLine("  货物描述：");
            int bt = readInt("  优化目标（0=最低费用 / 1=最短耗时）：");
            om.addOrder(Order{oid, src, dst, goods, bt != 0});

        } else if (ch == 2) {
            int oid = readInt("  要删除的订单号：");
            om.removeOrder(oid);

        } else if (ch == 3) {
            om.listOrders();

        } else if (ch == 4) {
            string path = readLine("  文件路径（回车默认 data/orders.txt）：");
            if (path.empty()) path = "data/orders.txt";
            FileManager::loadOrders(path, om);

        } else if (ch == 5) {
            auto plans = om.planAllOrders(g);
            cout << "\n共规划 " << plans.size() << " 条订单：\n";
            for (const auto& p : plans)
                om.printPlan(g, p);

        } else if (ch == 6) {
            auto topoRes = om.planBatchSequence(g);
            if (topoRes.hasCycle) {
                cout << "\n[警告] 配送网络存在环路，无法完成拓扑排序！\n";
                cout << "  涉及环路的网点编号：";
                for (int id : topoRes.cycleNodes) {
                    const Node* n = g.findNode(id);
                    cout << id << "(" << (n ? n->name : "?") << ") ";
                }
                cout << "\n";
            } else {
                cout << "\n批次配送顺序（拓扑排序结果）：\n  ";
                for (size_t i = 0; i < topoRes.order.size(); ++i) {
                    int id = topoRes.order[i];
                    const Node* n = g.findNode(id);
                    cout << id << "(" << (n ? n->name : "?") << ")";
                    if (i + 1 < topoRes.order.size()) cout << " -> ";
                }
                cout << "\n";
            }

        } else if (ch == 7) {
            string path = readLine("  保存路径（回车默认 data/plans.txt）：");
            if (path.empty()) path = "data/plans.txt";
            auto plans = om.planAllOrders(g);
            FileManager::savePlans(path, g, plans);
        }
        pause();
    }
}

// ─── 主菜单 ───────────────────────────────────────────
int main() {
    Logger::instance().setLogFile("logs/system.log");
    LOG_INFO("系统启动");

    // 自动加载默认路网
    if (FileManager::loadNetwork("data/network.txt", g)) {
        cout << "已自动加载路网：" << g.nodeCount() << " 个网点，"
             << g.edgeCount() << " 条路段\n";
    }

    while (true) {
        cout << "\n======== 快递网点配送路径规划系统 ========\n"
             << "  当前：" << g.nodeCount() << " 个网点  "
             << g.edgeCount() << " 条路段  "
             << om.getOrders().size() << " 条订单\n"
             << "-------------------------------------------\n"
             << "  1. 网点管理\n"
             << "  2. 路网管理\n"
             << "  3. 路径查询\n"
             << "  4. 批次配送\n"
             << "  0. 退出\n";
        int ch = readInt("请选择：");
        if (ch == 0) {
            LOG_INFO("系统正常退出");
            cout << "再见！\n";
            break;
        }
        switch (ch) {
            case 1: menuNode();     break;
            case 2: menuNetwork();  break;
            case 3: menuPath();     break;
            case 4: menuDelivery(); break;
            default: cout << "无效选项\n"; break;
        }
    }
    return 0;
}
