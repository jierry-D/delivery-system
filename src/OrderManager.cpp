// OrderManager + FileManager 合并实现
#include "../include/OrderManager.h"
#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using std::string; using std::to_string;

// ════════════════════════════════════════════════════
//  OrderManager
// ════════════════════════════════════════════════════
bool OrderManager::addOrder(const Order& o) {
    for (int i=0; i<orders_.size(); ++i)
        if (orders_[i].orderId==o.orderId) { LOG_WARN("订单"+to_string(o.orderId)+"已存在"); return false; }
    orders_.push_back(o);
    LOG_INFO("添加订单 "+to_string(o.orderId)+": "+to_string(o.srcNode)+"->"+to_string(o.dstNode));
    return true;
}

bool OrderManager::removeOrder(int id) {
    bool ok=orders_.remove_first([id](const Order& o){ return o.orderId==id; });
    if (!ok) { LOG_WARN("订单"+to_string(id)+"不存在"); return false; }
    LOG_INFO("删除订单 "+to_string(id)); return true;
}

void OrderManager::listOrders() const {
    if (orders_.empty()) { std::cout<<"  （当前无订单）\n"; return; }
    std::cout<<std::left<<std::setw(8)<<"订单号"<<std::setw(8)<<"起点"
             <<std::setw(8)<<"终点"<<std::setw(16)<<"货物"<<"优化目标\n"
             <<std::string(50,'-')<<"\n";
    for (int i=0;i<orders_.size();++i) {
        const Order& o=orders_[i];
        std::cout<<std::setw(8)<<o.orderId<<std::setw(8)<<o.srcNode
                 <<std::setw(8)<<o.dstNode<<std::setw(16)<<o.goods
                 <<(o.byTime?"最短耗时":"最低费用")<<"\n";
    }
}

DynArray<DeliveryPlan> OrderManager::planAllOrders(const Graph& g) const {
    DynArray<DeliveryPlan> plans;
    for (int i=0;i<orders_.size();++i) {
        DeliveryPlan p; p.order=orders_[i];
        if (orders_[i].byTime) {
            auto all=Dijkstra::shortestTimeFrom(g, orders_[i].srcNode);
            PathResult* pr=all.find(orders_[i].dstNode);
            p.result = pr ? *pr : PathResult{};
        } else {
            p.result = Dijkstra::cheapestPath(g, orders_[i].srcNode, orders_[i].dstNode);
        }
        plans.push_back(p);
    }
    LOG_INFO("批量规划完成，共"+to_string(plans.size())+"条");
    return plans;
}

TopoResult OrderManager::planBatchSequence(const Graph& g) const {
    HashMap<int,bool> seen; DynArray<int> ids;
    for (int i=0;i<orders_.size();++i) {
        if (!seen.contains(orders_[i].srcNode)) { seen.set(orders_[i].srcNode,true); ids.push_back(orders_[i].srcNode); }
        if (!seen.contains(orders_[i].dstNode)) { seen.set(orders_[i].dstNode,true); ids.push_back(orders_[i].dstNode); }
    }
    return TopoSort::sort(g, ids);
}

// ════════════════════════════════════════════════════
//  FileManager
// ════════════════════════════════════════════════════
bool FileManager::loadNetwork(const string& path, Graph& g) {
    std::ifstream fin(path);
    if (!fin) { LOG_ERROR("无法打开："+path); return false; }
    g.clear(); string line, sec; int nc=0,ec=0;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        if (line[0]=='#') {
            if (line.find("NODES")!=string::npos) sec="N";
            else if (line.find("EDGES")!=string::npos) sec="E";
            continue;
        }
        std::istringstream ss(line);
        if (sec=="N") {
            int id; string name,addr;
            if (ss>>id>>name) { std::getline(ss,addr); if(!addr.empty()&&addr[0]==' ') addr=addr.substr(1); if(g.addNode(Node(id,name,addr))) ++nc; }
        } else if (sec=="E") {
            int f,t; double ti,co;
            if (ss>>f>>t>>ti>>co && g.addEdge(Edge(f,t,ti,co))) ++ec;
        }
    }
    LOG_INFO("路网导入："+to_string(nc)+"网点 "+to_string(ec)+"路段 来自"+path);
    return true;
}

bool FileManager::saveNetwork(const string& path, const Graph& g) {
    std::ofstream fout(path);
    if (!fout) { LOG_ERROR("无法写入："+path); return false; }
    DynArray<int> ids=g.getAllNodeIds();
    for (int i=0;i<ids.size()-1;++i) for(int j=i+1;j<ids.size();++j) if(ids[i]>ids[j]){int t=ids[i];ids[i]=ids[j];ids[j]=t;}
    fout<<"# NODES\n";
    for (int i=0;i<ids.size();++i) { const Node* n=g.findNode(ids[i]); if(n) fout<<n->id<<" "<<n->name<<" "<<n->address<<"\n"; }
    fout<<"# EDGES\n";
    for (int i=0;i<ids.size();++i) {
        const DynArray<Edge>& es=g.getNeighbors(ids[i]);
        for (int j=0;j<es.size();++j) fout<<std::fixed<<std::setprecision(2)<<es[j].from<<" "<<es[j].to<<" "<<es[j].time<<" "<<es[j].cost<<"\n";
    }
    LOG_INFO("路网已保存："+path); return true;
}

bool FileManager::loadOrders(const string& path, OrderManager& om) {
    std::ifstream fin(path);
    if (!fin) { LOG_ERROR("无法打开订单文件："+path); return false; }
    int cnt=0; string line;
    while (std::getline(fin, line)) {
        if (line.empty()||line[0]=='#') continue;
        std::istringstream ss(line); int oid,src,dst,bt; string goods;
        if (ss>>oid>>src>>dst>>goods>>bt && om.addOrder({oid,src,dst,goods,bt!=0})) ++cnt;
    }
    LOG_INFO("导入订单："+to_string(cnt)+"条 来自"+path); return true;
}

bool FileManager::savePlans(const string& path, const Graph&, const DynArray<DeliveryPlan>& plans) {
    std::ofstream fout(path);
    if (!fout) { LOG_ERROR("无法写入配送方案："+path); return false; }
    fout<<std::fixed<<std::setprecision(2)<<"# 配送方案\n";
    for (int i=0;i<plans.size();++i) {
        const Order& o=plans[i].order; const PathResult& r=plans[i].result;
        fout<<o.orderId<<" "<<o.goods<<" "<<(o.byTime?"最短耗时":"最低费用")<<" ";
        if (!r.reachable) { fout<<"不可达\n"; continue; }
        fout<<r.totalTime<<" "<<r.totalCost<<" ";
        for (int j=0;j<r.path.size();++j) { fout<<r.path[j]; if(j+1<r.path.size()) fout<<"->"; }
        fout<<"\n";
    }
    LOG_INFO("方案已保存："+path); return true;
}
