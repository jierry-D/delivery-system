#include "../include/Graph.h"
#include "../include/Logger.h"
#include <iostream>
#include <iomanip>

using std::string; using std::to_string;
const DynArray<Edge> Graph::empty_;

bool Graph::addNode(const Node& n) {
    if (n.name.empty()) { LOG_WARN("添加网点失败：名称为空"); return false; }
    if (nodes.contains(n.id)) { LOG_WARN("添加网点失败：编号"+to_string(n.id)+"已存在"); return false; }
    nodes.set(n.id, n); adj[n.id];
    LOG_INFO("添加网点成功：["+to_string(n.id)+"] "+n.name);
    return true;
}

bool Graph::deleteNode(int id) {
    if (!nodes.contains(id)) { LOG_WARN("删除失败：网点"+to_string(id)+"不存在"); return false; }
    nodes.erase(id); adj.erase(id);
    adj.forEach([id](int, DynArray<Edge>& es) {
        es.remove_all([id](const Edge& e){ return e.to==id; });
    });
    LOG_INFO("删除网点："+to_string(id)); return true;
}

bool Graph::updateNode(int id, const Node& n) {
    if (!nodes.contains(id)) { LOG_WARN("修改失败：网点不存在"); return false; }
    if (n.name.empty())      { LOG_WARN("修改失败：名称为空");   return false; }
    Node u=n; u.id=id; nodes.set(id, u);
    LOG_INFO("修改网点："+to_string(id)); return true;
}

Node*       Graph::findNode(int id)       { return nodes.find(id); }
const Node* Graph::findNode(int id) const { return nodes.find(id); }

static DynArray<int> sortedIds(const Graph& g) {
    DynArray<int> ids = g.getAllNodeIds();
    for (int i=0;i<ids.size()-1;++i)
        for (int j=i+1;j<ids.size();++j)
            if (ids[i]>ids[j]) { int t=ids[i];ids[i]=ids[j];ids[j]=t; }
    return ids;
}

void Graph::listAllNodes() const {
    if (nodes.empty()) { std::cout<<"  （当前无网点）\n"; return; }
    std::cout<<std::left<<std::setw(6)<<"编号"<<std::setw(16)<<"名称"<<"地址\n"
             <<std::string(50,'-')<<"\n";
    for (int id : sortedIds(*this)) {
        const Node* n=findNode(id);
        if(n) std::cout<<std::setw(6)<<n->id<<std::setw(16)<<n->name<<n->address<<"\n";
    }
}

bool Graph::addEdge(const Edge& e) {
    if (!nodes.contains(e.from)||!nodes.contains(e.to))
        { LOG_WARN("添加路段失败：节点不存在"); return false; }
    if (e.from==e.to) { LOG_WARN("添加路段失败：不允许自环"); return false; }
    if (e.time<0||e.cost<0) { LOG_WARN("添加路段失败：权重为负"); return false; }
    DynArray<Edge>& es=adj[e.from];
    for (int i=0;i<es.size();++i) if(es[i].to==e.to) { LOG_WARN("路段已存在"); return false; }
    es.push_back(e);
    LOG_INFO("添加路段："+to_string(e.from)+"->"+to_string(e.to)+" "+to_string(e.time)+"h "+to_string(e.cost)+"元");
    return true;
}

bool Graph::deleteEdge(int from, int to) {
    DynArray<Edge>* ep=adj.find(from);
    if (!ep) { LOG_WARN("删除路段失败：起点不存在"); return false; }
    bool ok=ep->remove_first([to](const Edge& e){ return e.to==to; });
    if (!ok) { LOG_WARN("删除路段失败：路段不存在"); return false; }
    LOG_INFO("删除路段："+to_string(from)+"->"+to_string(to)); return true;
}

void Graph::listAllEdges() const {
    std::cout<<std::left<<std::setw(8)<<"起点"<<std::setw(8)<<"终点"
             <<std::setw(12)<<"耗时(h)"<<"费用(元)\n"<<std::string(45,'-')<<"\n";
    for (int id : sortedIds(*this)) {
        const DynArray<Edge>* ep=adj.find(id);
        if (!ep) continue;
        for (int j=0;j<ep->size();++j)
            std::cout<<std::setw(8)<<(*ep)[j].from<<std::setw(8)<<(*ep)[j].to
                     <<std::setw(12)<<(*ep)[j].time<<(*ep)[j].cost<<"\n";
    }
}

const DynArray<Edge>& Graph::getNeighbors(int id) const {
    const DynArray<Edge>* p=adj.find(id); return p ? *p : empty_;
}

DynArray<int> Graph::getAllNodeIds() const {
    DynArray<int> ids;
    nodes.forEach([&](int id, const Node&){ ids.push_back(id); });
    return ids;
}

bool Graph::hasNode(int id)  const { return nodes.contains(id); }
int  Graph::nodeCount()      const { return nodes.size(); }
int  Graph::edgeCount()      const {
    int c=0; adj.forEach([&](int, const DynArray<Edge>& e){ c+=e.size(); }); return c;
}
void Graph::clear() { nodes.clear(); adj.clear(); }
