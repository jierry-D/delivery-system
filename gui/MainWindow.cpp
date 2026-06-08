#include "MainWindow.h"
#include "../include/Dijkstra.h"
#include "../include/Logger.h"

#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QScrollBar>
#include <cmath>
#include <sstream>
#include <iomanip>

// ════════════════════════════════════════════════════
//  GraphWidget
// ════════════════════════════════════════════════════
static const struct { int id; float lon,lat; } GEO[25]={
    { 1,116.4f,39.9f},{ 2,117.2f,39.1f},{ 3,114.5f,38.0f},{ 4,112.5f,37.9f},{ 5,111.6f,40.8f},
    { 6,123.4f,41.8f},{ 7,121.6f,38.9f},{ 8,125.3f,43.9f},{ 9,126.5f,45.8f},{10,121.5f,31.2f},
    {11,118.8f,32.1f},{12,120.2f,30.3f},{13,117.3f,31.8f},{14,119.3f,26.1f},{15,115.9f,28.7f},
    {16,117.0f,36.7f},{17,113.6f,34.7f},{18,114.3f,30.6f},{19,112.9f,28.2f},{20,113.3f,23.1f},
    {21,114.1f,22.5f},{22,108.4f,22.8f},{23,104.1f,30.7f},{24,106.5f,29.6f},{25,108.9f,34.3f}
};

GraphWidget::GraphWidget(QWidget* p): QWidget(p) { setMouseTracking(true); setMinimumSize(500,400); }

void GraphWidget::setGraph(const Graph* g) { g_=g; updatePos(); update(); }
void GraphWidget::setHL(const DynArray<int>& n, const DynArray<int>& s, const DynArray<int>& d, int src, int dst)
    { hlN_=n; hlSrc_=s; hlDst_=d; srcHL_=src; dstHL_=dst; update(); }
void GraphWidget::clearHL() { hlN_.clear(); hlSrc_.clear(); hlDst_.clear(); srcHL_=dstHL_=-1; update(); }

void GraphWidget::updatePos() {
    float W=width()-20.f, H=height()-20.f;
    if (W<1||H<1) return;
    for (auto& g : GEO)
        pos_.set(g.id, QPointF(10+(g.lon-103.5f)/24.5f*W, H+10-(g.lat-21.5f)/25.0f*H));
}
void GraphWidget::resizeEvent(QResizeEvent*) { updatePos(); }

bool GraphWidget::isNodeHL(int id) const { for(int i=0;i<hlN_.size();++i) if(hlN_[i]==id) return true; return false; }
bool GraphWidget::isEdgeHL(int f,int t) const {
    for(int i=0;i<hlSrc_.size();++i) if(hlSrc_[i]==f&&hlDst_[i]==t) return true; return false;
}
int GraphWidget::nodeAt(QPoint p) const {
    for (auto& g:GEO) { const QPointF* q=pos_.find(g.id); if(!q) continue;
        float dx=p.x()-q->x(), dy=p.y()-q->y();
        if(std::sqrt(dx*dx+dy*dy)<=R+4) return g.id; }
    return -1;
}

void GraphWidget::drawEdge(QPainter& p, QPointF a, QPointF b, QColor col, float w, bool bi) {
    QPointF d=b-a; float len=std::sqrt(d.x()*d.x()+d.y()*d.y()); if(len<1) return;
    QPointF u(d.x()/len,d.y()/len), perp(-u.y(),u.x());
    float off=bi?4.f:0;
    QPointF p1=a+u*R+perp*off, p2=b-u*R+perp*off;
    p.setPen(QPen(col,w)); p.drawLine(p1,p2);
    // 箭头
    float sz=10; QPolygonF tri;
    tri<<p2<<(p2-u*sz+perp*(sz*.42f))<<(p2-u*sz-perp*(sz*.42f));
    p.setPen(Qt::NoPen); p.setBrush(col); p.drawPolygon(tri);
}

void GraphWidget::drawNode(QPainter& p, int id, QPointF pos, QColor col) {
    p.setPen(Qt::NoPen); p.setBrush(QColor(0,0,0,45));
    p.drawEllipse(pos+QPointF(2,2),(double)R+2,(double)R+2);
    p.setPen(QPen(Qt::white,1.5)); p.setBrush(col);
    p.drawEllipse(pos,(double)R,(double)R);
    QFont f=p.font(); f.setPointSize(10); f.setBold(true); p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(QRectF(pos.x()-R,pos.y()-R,R*2,R*2),Qt::AlignCenter,QString::number(id));
    if (g_) { const Node* n=g_->findNode(id); if(n&&!n->name.empty()){
        QString nm=QString::fromStdString(n->name); if(nm.size()>3) nm=nm.left(3);
        f.setPointSize(9); f.setBold(false); p.setFont(f);
        p.setPen(QColor(40,55,70));
        p.drawText(QRectF(pos.x()-30,pos.y()+R+1,60,16),Qt::AlignCenter,nm);
    }}
}

void GraphWidget::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(240,245,250));
    if (!g_||g_->nodeCount()==0) { p.setPen(QColor(150,160,170)); p.drawText(rect(),Qt::AlignCenter,"（路网为空）"); return; }
    DynArray<int> ids=g_->getAllNodeIds();
    // 画边
    for (int i=0;i<ids.size();++i) {
        const QPointF* pa=pos_.find(ids[i]); if(!pa) continue;
        const DynArray<Edge>& ns=g_->getNeighbors(ids[i]);
        for (int j=0;j<ns.size();++j) {
            const QPointF* pb=pos_.find(ns[j].to); if(!pb) continue;
            bool hl=isEdgeHL(ids[i],ns[j].to), bi=false;
            const DynArray<Edge>& rev=g_->getNeighbors(ns[j].to);
            for(int k=0;k<rev.size();++k) if(rev[k].to==ids[i]){bi=true;break;}
            drawEdge(p,*pa,*pb, hl?QColor(230,100,30):QColor(160,180,200,180), hl?3.f:1.5f, bi);
        }
    }
    // 画节点
    for (int i=0;i<ids.size();++i) {
        const QPointF* pos=pos_.find(ids[i]); if(!pos) continue;
        QColor col=QColor(74,144,217);
        if(ids[i]==srcHL_) col=QColor(39,174,96);
        else if(ids[i]==dstHL_) col=QColor(231,76,60);
        else if(isNodeHL(ids[i])) col=QColor(243,156,18);
        else if(ids[i]==hov_) col=QColor(100,180,255);
        drawNode(p,ids[i],*pos,col);
    }
    // 悬停提示
    if (hov_>0&&g_) { const Node* n=g_->findNode(hov_); const QPointF* pos=pos_.find(hov_);
        if(n&&pos) {
            float bx=pos->x()+R+8, by=pos->y()-35;
            if(bx+200>width()) bx=pos->x()-208; if(by<5) by=5;
            QRectF box(bx,by,200,70);
            p.setPen(Qt::NoPen); p.setBrush(QColor(18,32,50,220)); p.drawRoundedRect(box,6,6);
            p.setPen(QColor(70,130,190)); p.drawRoundedRect(box,6,6);
            QFont f=p.font(); f.setPointSize(9); p.setFont(f);
            p.setPen(QColor(90,200,255));
            p.drawText(box.adjusted(6,5,-4,-45),Qt::AlignLeft,QString("[%1] %2").arg(n->id).arg(QString::fromStdString(n->name)));
            p.setPen(QColor(175,195,215));
            p.drawText(box.adjusted(6,25,-4,-25),Qt::AlignLeft,QString::fromStdString(n->address));
            p.setPen(QColor(140,180,210));
            p.drawText(box.adjusted(6,48,-4,-5),Qt::AlignLeft,QString("出边: %1").arg(g_->getNeighbors(n->id).size()));
        }
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent* ev) {
    int id=nodeAt(ev->pos()); if(id!=hov_){hov_=id; emit nodeHovered(id); update();}
}
void GraphWidget::mousePressEvent(QMouseEvent* ev) {
    if(ev->button()==Qt::LeftButton){int id=nodeAt(ev->pos()); if(id>0) emit nodeClicked(id);}
}

// ════════════════════════════════════════════════════
//  MainWindow
// ════════════════════════════════════════════════════
static const char* BS = "background:#3498db;color:white;border:none;border-radius:4px;padding:6px;";
static const char* BB = "background:#607080;color:white;border:none;border-radius:4px;padding:6px;";

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    setWindowTitle("快递网点配送路径规划系统"); resize(1280,760); buildUi();
    if (FileManager::loadNetwork("data/network.txt", g_)) {
        msgOk("路网加载："+QString::number(g_.nodeCount())+"网点/"+QString::number(g_.edgeCount())+"路段");
        updateStats(); refresh();
    }
}

QPushButton* MainWindow::btn(const QString& t, bool back) {
    auto* b=new QPushButton(t); b->setFixedHeight(40); b->setStyleSheet(back?BB:BS); return b;
}

QWidget* MainWindow::makePage(std::initializer_list<std::pair<QString,void(MainWindow::*)()>> items) {
    auto* w=new QWidget; auto* l=new QVBoxLayout(w); l->setSpacing(6);
    for (auto& [text, slot] : items) { auto* b=btn(text, text.startsWith("←")); connect(b,&QPushButton::clicked,this,slot); l->addWidget(b); }
    l->addStretch(); return w;
}

void MainWindow::buildUi() {
    // 菜单
    auto* fm=menuBar()->addMenu("文件");
    connect(fm->addAction("导入路网..."),   &QAction::triggered, this, &MainWindow::onImportNet);
    connect(fm->addAction("导出路网..."),   &QAction::triggered, this, &MainWindow::onExportNet);
    connect(fm->addAction("导入订单..."),   &QAction::triggered, this, &MainWindow::onImportOrd);
    connect(fm->addAction("导出方案..."),   &QAction::triggered, this, &MainWindow::onExportPlans);
    connect(fm->addAction("退出"),          &QAction::triggered, this, &QMainWindow::close);

    auto* central=new QWidget(this); setCentralWidget(central);
    auto* ml=new QHBoxLayout(central); ml->setContentsMargins(0,0,0,0); ml->setSpacing(0);

    // 左侧面板
    auto* left=new QWidget; left->setFixedWidth(260);
    left->setStyleSheet("background:#1e2d3d;");
    auto* ll=new QVBoxLayout(left); ll->setContentsMargins(8,8,8,8); ll->setSpacing(6);

    auto* title=new QLabel("快递路径规划系统");
    title->setStyleSheet("color:#5ab4ff;font-size:15px;font-weight:bold;padding:6px 0;");
    title->setAlignment(Qt::AlignCenter); ll->addWidget(title);

    stats_=new QLabel; stats_->setStyleSheet("color:#56d78a;font-size:12px;background:#16263a;border-radius:4px;padding:6px;");
    stats_->setAlignment(Qt::AlignCenter); ll->addWidget(stats_);

    mode_=new QLabel("▶ 主菜单"); mode_->setStyleSheet("color:#a0c8f0;font-size:12px;padding:2px 4px;"); ll->addWidget(mode_);

    stack_=new QStackedWidget;
    stack_->addWidget(makePage({{"网点管理",&MainWindow::goNode},{"路网管理",&MainWindow::goNetwork},{"路径查询",&MainWindow::goPath},{"批次配送",&MainWindow::goDelivery}}));
    stack_->addWidget(makePage({{"添加网点",&MainWindow::onAddNode},{"删除网点",&MainWindow::onDeleteNode},{"修改网点",&MainWindow::onUpdateNode},{"查询网点",&MainWindow::onFindNode},{"显示所有网点",&MainWindow::onListNodes},{"← 返回",&MainWindow::goMain}}));
    stack_->addWidget(makePage({{"添加路段",&MainWindow::onAddEdge},{"删除路段",&MainWindow::onDeleteEdge},{"显示所有路段",&MainWindow::onListEdges},{"导入路网",&MainWindow::onImportNet},{"导出路网",&MainWindow::onExportNet},{"← 返回",&MainWindow::goMain}}));
    stack_->addWidget(makePage({{"单源最短耗时路径",&MainWindow::onShortTime},{"两点最低费用路径",&MainWindow::onCheapPath},{"清除高亮",&MainWindow::onClearHL},{"← 返回",&MainWindow::goMain}}));
    stack_->addWidget(makePage({{"添加配送订单",&MainWindow::onAddOrder},{"删除订单",&MainWindow::onDelOrder},{"显示所有订单",&MainWindow::onListOrders},{"批量导入订单",&MainWindow::onImportOrd},{"规划所有订单",&MainWindow::onPlanAll},{"拓扑批次排序",&MainWindow::onTopoSort},{"导出配送方案",&MainWindow::onExportPlans},{"← 返回",&MainWindow::goMain}}));
    ll->addWidget(stack_);

    ll->addWidget([]{ auto* l=new QLabel("─── 操作日志 ───"); l->setStyleSheet("color:#3a5060;font-size:11px;"); return l; }());
    log_=new QTextEdit; log_->setReadOnly(true); log_->setMaximumHeight(180);
    log_->setStyleSheet("background:#0f1a26;color:#b0c8e0;font-size:12px;border:1px solid #243040;border-radius:3px;");
    ll->addWidget(log_);
    ml->addWidget(left);

    canvas_=new GraphWidget;
    connect(canvas_,&GraphWidget::nodeHovered,this,[this](int id){
        if(id>0){const Node* n=g_.findNode(id); if(n) statusBar()->showMessage(QString("[%1] %2  %3").arg(id).arg(QString::fromStdString(n->name)).arg(QString::fromStdString(n->address)));}
        else statusBar()->clearMessage();
    });
    ml->addWidget(canvas_,1);
    updateStats();
}

void MainWindow::goMain()     { stack_->setCurrentIndex(0); mode_->setText("▶ 主菜单");  }
void MainWindow::goNode()     { stack_->setCurrentIndex(1); mode_->setText("▶ 网点管理"); }
void MainWindow::goNetwork()  { stack_->setCurrentIndex(2); mode_->setText("▶ 路网管理"); }
void MainWindow::goPath()     { stack_->setCurrentIndex(3); mode_->setText("▶ 路径查询"); }
void MainWindow::goDelivery() { stack_->setCurrentIndex(4); mode_->setText("▶ 批次配送"); }

void MainWindow::updateStats() {
    stats_->setText(QString("网点：%1  路段：%2  订单：%3").arg(g_.nodeCount()).arg(g_.edgeCount()).arg(om_.getOrders().size()));
}
void MainWindow::refresh() { canvas_->setGraph(&g_); canvas_->setHL(hlN_,hlSrc_,hlDst_,srcHL_,dstHL_); }
void MainWindow::msg(const QString& t, const QString& c) {
    log_->append(QString("<span style='color:%1'>%2</span>").arg(c).arg(t));
    log_->verticalScrollBar()->setValue(log_->verticalScrollBar()->maximum());
}
void MainWindow::msgOk(const QString& t)  { msg(t,"#56d78a"); }
void MainWindow::msgErr(const QString& t) { msg(t,"#e05050"); }

void MainWindow::applyPath(const DynArray<int>& path) {
    for (int i=0;i+1<path.size();++i){ hlSrc_.push_back(path[i]); hlDst_.push_back(path[i+1]); hlN_.push_back(path[i]); hlN_.push_back(path[i+1]); }
    refresh();
}
void MainWindow::clearHL() { hlN_.clear(); hlSrc_.clear(); hlDst_.clear(); srcHL_=dstHL_=-1; canvas_->clearHL(); }

// ── 对话框辅助 ────────────────────────────────────────────
#define MAKE_DLG(title) QDialog dlg(this); dlg.setWindowTitle(title); auto* form=new QFormLayout(&dlg); \
    auto* btns=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); \
    connect(btns,&QDialogButtonBox::accepted,&dlg,&QDialog::accept); \
    connect(btns,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
#define SPIN(var,lo,hi) auto* var=new QSpinBox; var->setRange(lo,hi);
#define DSPIN(var,lo,hi) auto* var=new QDoubleSpinBox; var->setRange(lo,hi); var->setDecimals(1);

// ── 网点管理 ──────────────────────────────────────────────
void MainWindow::onAddNode() {
    MAKE_DLG("添加网点"); SPIN(id,1,9999); auto* nm=new QLineEdit; auto* addr=new QLineEdit;
    form->addRow("编号:",id); form->addRow("名称:",nm); form->addRow("地址:",addr); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(g_.addNode(Node(id->value(),nm->text().toStdString(),addr->text().toStdString())))
        { msgOk("网点 "+QString::number(id->value())+" 添加成功"); updateStats(); refresh(); }
    else msgErr("添加失败（编号已存在或名称为空）");
}
void MainWindow::onDeleteNode() {
    MAKE_DLG("删除网点"); SPIN(id,1,9999); form->addRow("编号:",id); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(g_.deleteNode(id->value())) { msgOk("网点 "+QString::number(id->value())+" 已删除"); clearHL(); updateStats(); refresh(); }
    else msgErr("网点不存在");
}
void MainWindow::onUpdateNode() {
    MAKE_DLG("修改网点"); SPIN(id,1,9999); auto* nm=new QLineEdit; auto* addr=new QLineEdit;
    form->addRow("编号:",id); form->addRow("新名称:",nm); form->addRow("新地址:",addr); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(g_.updateNode(id->value(),Node(id->value(),nm->text().toStdString(),addr->text().toStdString())))
        { msgOk("修改成功"); refresh(); }
    else msgErr("修改失败");
}
void MainWindow::onFindNode() {
    MAKE_DLG("查询网点"); SPIN(id,1,9999); form->addRow("编号:",id); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    const Node* n=g_.findNode(id->value());
    if(n){ msgOk(QString("[%1] %2  %3").arg(n->id).arg(QString::fromStdString(n->name)).arg(QString::fromStdString(n->address)));
           clearHL(); hlN_.push_back(n->id); refresh(); }
    else msgErr("网点不存在");
}
void MainWindow::onListNodes() {
    msg("── 所有网点（"+QString::number(g_.nodeCount())+"）──","#5ab4ff");
    DynArray<int> ids=g_.getAllNodeIds();
    for(int i=0;i<ids.size()-1;++i) for(int j=i+1;j<ids.size();++j) if(ids[i]>ids[j]){int t=ids[i];ids[i]=ids[j];ids[j]=t;}
    for(int i=0;i<ids.size();++i){ const Node* n=g_.findNode(ids[i]); if(n) msg(QString("[%1] %2  %3").arg(n->id).arg(QString::fromStdString(n->name)).arg(QString::fromStdString(n->address))); }
}

// ── 路网管理 ──────────────────────────────────────────────
void MainWindow::onAddEdge() {
    MAKE_DLG("添加路段"); SPIN(f,1,9999); SPIN(t,1,9999); DSPIN(ti,0,999); DSPIN(co,0,99999);
    form->addRow("起点:",f); form->addRow("终点:",t); form->addRow("耗时(h):",ti); form->addRow("费用(元):",co); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(g_.addEdge(Edge(f->value(),t->value(),ti->value(),co->value())))
        { msgOk(QString("路段 %1→%2 添加成功").arg(f->value()).arg(t->value())); updateStats(); refresh(); }
    else msgErr("添加失败");
}
void MainWindow::onDeleteEdge() {
    MAKE_DLG("删除路段"); SPIN(f,1,9999); SPIN(t,1,9999);
    form->addRow("起点:",f); form->addRow("终点:",t); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(g_.deleteEdge(f->value(),t->value())) { msgOk(QString("路段 %1→%2 已删除").arg(f->value()).arg(t->value())); clearHL(); updateStats(); refresh(); }
    else msgErr("路段不存在");
}
void MainWindow::onListEdges() {
    msg("── 所有路段（"+QString::number(g_.edgeCount())+"）──","#5ab4ff");
    DynArray<int> ids=g_.getAllNodeIds();
    for(int i=0;i<ids.size()-1;++i) for(int j=i+1;j<ids.size();++j) if(ids[i]>ids[j]){int t=ids[i];ids[i]=ids[j];ids[j]=t;}
    for(int i=0;i<ids.size();++i){ const DynArray<Edge>& es=g_.getNeighbors(ids[i]);
        for(int j=0;j<es.size();++j){ std::ostringstream s; s<<std::fixed<<std::setprecision(1)<<es[j].from<<"→"<<es[j].to<<"  "<<es[j].time<<"h  "<<es[j].cost<<"元"; msg(QString::fromStdString(s.str())); }
    }
}
void MainWindow::onImportNet() {
    QString p=QFileDialog::getOpenFileName(this,"导入路网","data","文本文件 (*.txt)"); if(p.isEmpty()) return;
    g_.clear(); clearHL();
    if(FileManager::loadNetwork(p.toStdString(),g_)) { msgOk("导入成功："+QString::number(g_.nodeCount())+"网点/"+QString::number(g_.edgeCount())+"路段"); updateStats(); refresh(); }
    else msgErr("导入失败");
}
void MainWindow::onExportNet() {
    QString p=QFileDialog::getSaveFileName(this,"导出路网","data/network.txt","文本文件 (*.txt)"); if(p.isEmpty()) return;
    FileManager::saveNetwork(p.toStdString(),g_) ? msgOk("已保存："+p) : msgErr("保存失败");
}

// ── 路径查询 ──────────────────────────────────────────────
void MainWindow::onShortTime() {
    MAKE_DLG("单源最短耗时"); SPIN(src,1,9999); form->addRow("起点:",src); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    clearHL(); auto res=Dijkstra::shortestTimeFrom(g_,src->value());
    if(res.empty()){ msgErr("起点不存在"); return; }
    srcHL_=src->value(); msg("── 从["+QString::number(src->value())+"]最短耗时 ──","#5ac4ff");
    DynArray<int> ids=g_.getAllNodeIds();
    for(int i=0;i<ids.size();++i){ if(ids[i]==src->value()) continue;
        PathResult* pr=res.find(ids[i]); if(!pr||!pr->reachable) continue;
        for(int j=0;j+1<pr->path.size();++j){ hlSrc_.push_back(pr->path[j]); hlDst_.push_back(pr->path[j+1]); }
        hlN_.push_back(ids[i]);
        std::ostringstream s; s<<std::fixed<<std::setprecision(1); const Node* n=g_.findNode(ids[i]);
        s<<"["<<ids[i]<<"]"<<(n?n->name:"")<<"  "<<pr->totalTime<<"h / "<<pr->totalCost<<"元";
        msg(QString::fromStdString(s.str()),"#c8deb0");
    }
    refresh();
}
void MainWindow::onCheapPath() {
    MAKE_DLG("两点最低费用路径"); SPIN(s,1,9999); SPIN(d,1,9999);
    form->addRow("起点:",s); form->addRow("终点:",d); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    clearHL(); PathResult r=Dijkstra::cheapestPath(g_,s->value(),d->value());
    if(!r.reachable){ msgErr("不可达"); return; }
    srcHL_=s->value(); dstHL_=d->value(); applyPath(r.path);
    std::ostringstream ss; ss<<std::fixed<<std::setprecision(1);
    ss<<"路径: "; for(int i=0;i<r.path.size();++i){ ss<<r.path[i]; if(i+1<r.path.size()) ss<<"→"; }
    msgOk(QString::fromStdString(ss.str())); ss.str(""); ss<<"费用: "<<r.totalCost<<"元  耗时: "<<r.totalTime<<"h"; msgOk(QString::fromStdString(ss.str()));
}
void MainWindow::onClearHL() { clearHL(); msgOk("已清除高亮"); }

// ── 批次配送 ──────────────────────────────────────────────
void MainWindow::onAddOrder() {
    MAKE_DLG("添加配送订单"); SPIN(oid,1,99999); SPIN(s,1,9999); SPIN(d,1,9999);
    auto* goods=new QLineEdit; auto* opt=new QComboBox; opt->addItem("最低费用"); opt->addItem("最短耗时");
    form->addRow("订单号:",oid); form->addRow("起点:",s); form->addRow("终点:",d);
    form->addRow("货物:",goods); form->addRow("优化目标:",opt); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    if(om_.addOrder({oid->value(),s->value(),d->value(),goods->text().toStdString(),opt->currentIndex()==1}))
        { msgOk("订单 "+QString::number(oid->value())+" 添加成功"); updateStats(); }
    else msgErr("订单号已存在");
}
void MainWindow::onDelOrder() {
    MAKE_DLG("删除订单"); SPIN(id,1,99999); form->addRow("订单号:",id); form->addRow(btns);
    if(dlg.exec()!=QDialog::Accepted) return;
    om_.removeOrder(id->value()) ? (msgOk("订单 "+QString::number(id->value())+" 已删除"),updateStats()) : msgErr("订单不存在");
}
void MainWindow::onListOrders() {
    const DynArray<Order>& os=om_.getOrders();
    msg("── 所有订单（"+QString::number(os.size())+"）──","#5ab4ff");
    for(int i=0;i<os.size();++i) msg(QString("[%1] %2→%3  %4  %5").arg(os[i].orderId).arg(os[i].srcNode).arg(os[i].dstNode).arg(QString::fromStdString(os[i].goods)).arg(os[i].byTime?"最短耗时":"最低费用"));
}
void MainWindow::onImportOrd() {
    QString p=QFileDialog::getOpenFileName(this,"导入订单","data","文本文件 (*.txt)"); if(p.isEmpty()) return;
    if(FileManager::loadOrders(p.toStdString(),om_)) { msgOk("导入："+QString::number(om_.getOrders().size())+"条"); updateStats(); }
    else msgErr("导入失败");
}
void MainWindow::onPlanAll() {
    clearHL(); DynArray<DeliveryPlan> plans=om_.planAllOrders(g_);
    msg("── 批量规划结果 ──","#5ac4ff");
    for(int i=0;i<plans.size();++i){ const DeliveryPlan& p=plans[i];
        if(!p.result.reachable){ msgErr("订单 "+QString::number(p.order.orderId)+" 不可达"); continue; }
        std::ostringstream s; s<<std::fixed<<std::setprecision(1)<<"["<<p.order.orderId<<"] "<<p.order.goods<<": ";
        for(int j=0;j<p.result.path.size();++j){ s<<p.result.path[j]; if(j+1<p.result.path.size()) s<<"→"; }
        s<<"  "<<(p.order.byTime?p.result.totalTime:p.result.totalCost)<<(p.order.byTime?"h":"元");
        msgOk(QString::fromStdString(s.str())); applyPath(p.result.path);
    }
    msgOk("完成，共 "+QString::number(plans.size())+" 条");
}
void MainWindow::onTopoSort() {
    clearHL(); TopoResult res=om_.planBatchSequence(g_);
    if(res.hasCycle){ msgErr("检测到环路！涉及节点：");
        for(int i=0;i<res.cycleNodes.size();++i){ hlN_.push_back(res.cycleNodes[i]); const Node* n=g_.findNode(res.cycleNodes[i]); msg(QString("  %1（%2）").arg(res.cycleNodes[i]).arg(n?QString::fromStdString(n->name):"?"),"#e09050"); }
    } else { msg("── 拓扑排序配送顺序 ──","#5ac4ff");
        QString line;
        for(int i=0;i<res.order.size();++i){ int id=res.order[i]; hlN_.push_back(id);
            const Node* n=g_.findNode(id); QString nm=n?QString::fromStdString(n->name):"?"; if(nm.size()>3) nm=nm.left(3);
            line+=QString("%1(%2)").arg(id).arg(nm); if(i+1<res.order.size()) line+="→";
            if(line.size()>48){ msg(line,"#c8d8b0"); line.clear(); }
        }
        if(!line.isEmpty()) msg(line,"#c8d8b0");
        msgOk(QString::number(res.order.size())+" 个节点，无环路");
    }
    refresh();
}
void MainWindow::onExportPlans() {
    QString p=QFileDialog::getSaveFileName(this,"导出方案","data/plans.txt","文本文件 (*.txt)"); if(p.isEmpty()) return;
    DynArray<DeliveryPlan> plans=om_.planAllOrders(g_);
    FileManager::savePlans(p.toStdString(),g_,plans) ? msgOk("已导出："+p) : msgErr("导出失败");
}
