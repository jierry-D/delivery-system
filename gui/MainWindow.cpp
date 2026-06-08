#include "MainWindow.h"
#include "../include/FileManager.h"
#include "../include/Dijkstra.h"
#include "../include/Logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QScrollBar>
#include <sstream>
#include <iomanip>
#include <string>

using std::to_string;

// ── 颜色常量 ─────────────────────────────────────────────
static const char* PANEL_BG  = "#1e2d3d";
static const char* BTN_BLUE  = "background:#3498db;color:white;border:none;border-radius:4px;padding:6px;";
static const char* BTN_BACK  = "background:#607080;color:white;border:none;border-radius:4px;padding:6px;";

// ── 构造 & UI 初始化 ──────────────────────────────────────
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("快递网点配送路径规划系统");
    resize(1280, 760);
    setupUi();

    // 自动加载默认路网
    if (FileManager::loadNetwork("data/network.txt", _graph)) {
        msgOk("路网加载：" + QString::number(_graph.nodeCount()) +
              " 网点 / " + QString::number(_graph.edgeCount()) + " 路段");
        updateStats();
        refreshCanvas();
    }
}

void MainWindow::setupUi() {
    // ── 菜单栏 ──────────────────────────────────────────
    auto* fileMenu = menuBar()->addMenu("文件");
    connect(fileMenu->addAction("导入路网..."),  &QAction::triggered, this, &MainWindow::onImportNetwork);
    connect(fileMenu->addAction("导出路网..."),  &QAction::triggered, this, &MainWindow::onExportNetwork);
    fileMenu->addSeparator();
    connect(fileMenu->addAction("导入订单..."),  &QAction::triggered, this, &MainWindow::onImportOrders);
    connect(fileMenu->addAction("导出配送方案..."), &QAction::triggered, this, &MainWindow::onExportPlans);
    fileMenu->addSeparator();
    connect(fileMenu->addAction("退出"), &QAction::triggered, this, &QMainWindow::close);

    // ── 中央布局 ────────────────────────────────────────
    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── 左侧面板 ────────────────────────────────────────
    auto* leftPanel = new QWidget;
    leftPanel->setFixedWidth(260);
    leftPanel->setStyleSheet(QString("background:%1;").arg(PANEL_BG));
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(6);

    // 标题
    auto* title = new QLabel("快递路径规划系统");
    title->setStyleSheet("color:#5ab4ff;font-size:15px;font-weight:bold;padding:6px 0;");
    title->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(title);

    // 统计信息
    _stats = new QLabel;
    _stats->setStyleSheet("color:#56d78a;font-size:12px;background:#16263a;border-radius:4px;padding:6px;");
    _stats->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(_stats);

    // 当前模式标签
    _modeLabel = new QLabel("▶ 主菜单");
    _modeLabel->setStyleSheet("color:#a0c8f0;font-size:12px;padding:2px 4px;");
    leftLayout->addWidget(_modeLabel);

    // 按钮堆栈
    _stack = new QStackedWidget;
    _stack->addWidget(makeMainPage());      // 0: 主菜单
    _stack->addWidget(makeNodePage());      // 1: 网点管理
    _stack->addWidget(makeNetworkPage());   // 2: 路网管理
    _stack->addWidget(makePathPage());      // 3: 路径查询
    _stack->addWidget(makeDeliveryPage());  // 4: 批次配送
    leftLayout->addWidget(_stack);

    // 消息日志
    auto* logLabel = new QLabel("─── 操作日志 ───");
    logLabel->setStyleSheet("color:#3a5060;font-size:11px;");
    leftLayout->addWidget(logLabel);

    _log = new QTextEdit;
    _log->setReadOnly(true);
    _log->setStyleSheet("background:#0f1a26;color:#b0c8e0;font-size:12px;"
                        "border:1px solid #243040;border-radius:3px;");
    _log->setMaximumHeight(180);
    leftLayout->addWidget(_log);

    mainLayout->addWidget(leftPanel);

    // ── 右侧画布 ─────────────────────────────────────────
    _canvas = new GraphWidget;
    connect(_canvas, &GraphWidget::nodeHovered, this, [this](int id) {
        if (id > 0) {
            const Node* n = _graph.findNode(id);
            if (n) statusBar()->showMessage(
                QString("[%1] %2  %3").arg(id)
                    .arg(QString::fromStdString(n->name))
                    .arg(QString::fromStdString(n->address)));
        } else {
            statusBar()->clearMessage();
        }
    });
    mainLayout->addWidget(_canvas, 1);

    updateStats();
}

// ── 页面构建 ──────────────────────────────────────────────
QPushButton* MainWindow::makeBtn(const QString& text, bool isBack) {
    auto* btn = new QPushButton(text);
    btn->setFixedHeight(40);
    btn->setStyleSheet(isBack ? BTN_BACK : BTN_BLUE);
    return btn;
}

QWidget* MainWindow::makeMainPage() {
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    l->setSpacing(6);
    auto add = [&](const QString& t, auto slot) {
        auto* b = makeBtn(t);
        connect(b, &QPushButton::clicked, this, slot);
        l->addWidget(b);
    };
    add("网点管理", &MainWindow::goNode);
    add("路网管理", &MainWindow::goNetwork);
    add("路径查询", &MainWindow::goPath);
    add("批次配送", &MainWindow::goDelivery);
    l->addStretch();
    return w;
}

QWidget* MainWindow::makeNodePage() {
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    l->setSpacing(6);
    auto add = [&](const QString& t, auto slot, bool back = false) {
        auto* b = makeBtn(t, back);
        connect(b, &QPushButton::clicked, this, slot);
        l->addWidget(b);
    };
    add("添加网点",   &MainWindow::onAddNode);
    add("删除网点",   &MainWindow::onDeleteNode);
    add("修改网点",   &MainWindow::onUpdateNode);
    add("查询网点",   &MainWindow::onFindNode);
    add("显示所有网点", &MainWindow::onListNodes);
    add("← 返回",    &MainWindow::goMain, true);
    l->addStretch();
    return w;
}

QWidget* MainWindow::makeNetworkPage() {
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    l->setSpacing(6);
    auto add = [&](const QString& t, auto slot, bool back = false) {
        auto* b = makeBtn(t, back);
        connect(b, &QPushButton::clicked, this, slot);
        l->addWidget(b);
    };
    add("添加路段",    &MainWindow::onAddEdge);
    add("删除路段",    &MainWindow::onDeleteEdge);
    add("显示所有路段", &MainWindow::onListEdges);
    add("导入路网文件", &MainWindow::onImportNetwork);
    add("导出路网文件", &MainWindow::onExportNetwork);
    add("← 返回",     &MainWindow::goMain, true);
    l->addStretch();
    return w;
}

QWidget* MainWindow::makePathPage() {
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    l->setSpacing(6);
    auto add = [&](const QString& t, auto slot, bool back = false) {
        auto* b = makeBtn(t, back);
        connect(b, &QPushButton::clicked, this, slot);
        l->addWidget(b);
    };
    add("单源最短耗时路径",   &MainWindow::onShortestTime);
    add("两点最低费用路径",   &MainWindow::onCheapestPath);
    add("清除高亮",           &MainWindow::onClearHL);
    add("← 返回",            &MainWindow::goMain, true);
    l->addStretch();
    return w;
}

QWidget* MainWindow::makeDeliveryPage() {
    auto* w = new QWidget;
    auto* l = new QVBoxLayout(w);
    l->setSpacing(6);
    auto add = [&](const QString& t, auto slot, bool back = false) {
        auto* b = makeBtn(t, back);
        connect(b, &QPushButton::clicked, this, slot);
        l->addWidget(b);
    };
    add("添加配送订单",   &MainWindow::onAddOrder);
    add("删除订单",       &MainWindow::onRemoveOrder);
    add("显示所有订单",   &MainWindow::onListOrders);
    add("批量导入订单",   &MainWindow::onImportOrders);
    add("规划所有订单",   &MainWindow::onPlanAll);
    add("拓扑批次排序",   &MainWindow::onTopoSort);
    add("导出配送方案",   &MainWindow::onExportPlans);
    add("← 返回",        &MainWindow::goMain, true);
    l->addStretch();
    return w;
}

// ── 导航 ────────────────────────────────────────────────
void MainWindow::goMain()     { _stack->setCurrentIndex(0); _modeLabel->setText("▶ 主菜单");  }
void MainWindow::goNode()     { _stack->setCurrentIndex(1); _modeLabel->setText("▶ 网点管理"); }
void MainWindow::goNetwork()  { _stack->setCurrentIndex(2); _modeLabel->setText("▶ 路网管理"); }
void MainWindow::goPath()     { _stack->setCurrentIndex(3); _modeLabel->setText("▶ 路径查询"); }
void MainWindow::goDelivery() { _stack->setCurrentIndex(4); _modeLabel->setText("▶ 批次配送"); }

// ── 工具方法 ──────────────────────────────────────────────
void MainWindow::updateStats() {
    _stats->setText(QString("网点：%1    路段：%2    订单：%3")
        .arg(_graph.nodeCount())
        .arg(_graph.edgeCount())
        .arg(_om.getOrders().size()));
}

void MainWindow::refreshCanvas() {
    _canvas->setGraph(&_graph);
    _canvas->setHighlight(_hlNodes, _hlEdgeSrc, _hlEdgeDst, _srcHl, _dstHl);
}

void MainWindow::msg(const QString& text, const QString& color) {
    _log->append(QString("<span style='color:%1'>%2</span>").arg(color).arg(text));
    _log->verticalScrollBar()->setValue(_log->verticalScrollBar()->maximum());
}
void MainWindow::msgOk (const QString& t) { msg(t, "#56d78a"); }
void MainWindow::msgErr(const QString& t) { msg(t, "#e05050"); }

void MainWindow::applyPath(const DynArray<int>& path) {
    for (int i = 0; i + 1 < path.size(); ++i) {
        _hlEdgeSrc.push_back(path[i]);
        _hlEdgeDst.push_back(path[i + 1]);
        _hlNodes.push_back(path[i]);
        _hlNodes.push_back(path[i + 1]);
    }
    refreshCanvas();
}

void MainWindow::clearHL() {
    _hlNodes.clear(); _hlEdgeSrc.clear(); _hlEdgeDst.clear();
    _srcHl = _dstHl = -1;
    _canvas->clearHighlight();
}

// ── 网点管理 ──────────────────────────────────────────────
void MainWindow::onAddNode() {
    QDialog dlg(this);
    dlg.setWindowTitle("添加网点");
    auto* form = new QFormLayout(&dlg);
    auto* idSpin = new QSpinBox; idSpin->setRange(1, 9999);
    auto* nameEdit = new QLineEdit;
    auto* addrEdit = new QLineEdit;
    form->addRow("网点编号：", idSpin);
    form->addRow("名称：",     nameEdit);
    form->addRow("地址：",     addrEdit);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    Node n(idSpin->value(), nameEdit->text().toStdString(), addrEdit->text().toStdString());
    if (_graph.addNode(n)) {
        msgOk("网点 " + QString::number(n.id) + " 添加成功");
        updateStats(); refreshCanvas();
    } else {
        msgErr("添加失败（编号已存在或名称为空）");
    }
}

void MainWindow::onDeleteNode() {
    QDialog dlg(this);
    dlg.setWindowTitle("删除网点");
    auto* form = new QFormLayout(&dlg);
    auto* idSpin = new QSpinBox; idSpin->setRange(1, 9999);
    form->addRow("网点编号：", idSpin);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    int id = idSpin->value();
    if (_graph.deleteNode(id)) {
        msgOk("网点 " + QString::number(id) + " 已删除");
        clearHL(); updateStats(); refreshCanvas();
    } else {
        msgErr("网点 " + QString::number(id) + " 不存在");
    }
}

void MainWindow::onUpdateNode() {
    QDialog dlg(this);
    dlg.setWindowTitle("修改网点");
    auto* form = new QFormLayout(&dlg);
    auto* idSpin   = new QSpinBox; idSpin->setRange(1, 9999);
    auto* nameEdit = new QLineEdit;
    auto* addrEdit = new QLineEdit;
    form->addRow("网点编号：", idSpin);
    form->addRow("新名称：",   nameEdit);
    form->addRow("新地址：",   addrEdit);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    int id = idSpin->value();
    Node n(id, nameEdit->text().toStdString(), addrEdit->text().toStdString());
    if (_graph.updateNode(id, n)) {
        msgOk("网点 " + QString::number(id) + " 修改成功");
        refreshCanvas();
    } else {
        msgErr("修改失败（网点不存在或名称为空）");
    }
}

void MainWindow::onFindNode() {
    QDialog dlg(this);
    dlg.setWindowTitle("查询网点");
    auto* form = new QFormLayout(&dlg);
    auto* idSpin = new QSpinBox; idSpin->setRange(1, 9999);
    form->addRow("网点编号：", idSpin);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    int id = idSpin->value();
    const Node* n = _graph.findNode(id);
    if (n) {
        msgOk(QString("[%1] %2  %3").arg(id)
              .arg(QString::fromStdString(n->name))
              .arg(QString::fromStdString(n->address)));
        clearHL();
        _hlNodes.push_back(id);
        refreshCanvas();
    } else {
        msgErr("网点 " + QString::number(id) + " 不存在");
    }
}

void MainWindow::onListNodes() {
    msg("── 所有网点（" + QString::number(_graph.nodeCount()) + "）──", "#5ab4ff");
    DynArray<int> ids = _graph.getAllNodeIds();
    // 排序
    for (int i = 0; i < ids.size() - 1; ++i)
        for (int j = i+1; j < ids.size(); ++j)
            if (ids[i] > ids[j]) { int t=ids[i]; ids[i]=ids[j]; ids[j]=t; }
    for (int i = 0; i < ids.size(); ++i) {
        const Node* n = _graph.findNode(ids[i]);
        if (n) msg(QString("[%1] %2  %3").arg(n->id)
                   .arg(QString::fromStdString(n->name))
                   .arg(QString::fromStdString(n->address)));
    }
}

// ── 路网管理 ──────────────────────────────────────────────
void MainWindow::onAddEdge() {
    QDialog dlg(this);
    dlg.setWindowTitle("添加路段");
    auto* form = new QFormLayout(&dlg);
    auto* fromSpin = new QSpinBox; fromSpin->setRange(1, 9999);
    auto* toSpin   = new QSpinBox; toSpin->setRange(1, 9999);
    auto* timeSpin = new QDoubleSpinBox; timeSpin->setRange(0, 999); timeSpin->setDecimals(1);
    auto* costSpin = new QDoubleSpinBox; costSpin->setRange(0, 99999); costSpin->setDecimals(1);
    form->addRow("起点编号：",   fromSpin);
    form->addRow("终点编号：",   toSpin);
    form->addRow("耗时（小时）：", timeSpin);
    form->addRow("费用（元）：",  costSpin);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    Edge e(fromSpin->value(), toSpin->value(), timeSpin->value(), costSpin->value());
    if (_graph.addEdge(e)) {
        msgOk(QString("路段 %1→%2 添加成功").arg(e.from).arg(e.to));
        updateStats(); refreshCanvas();
    } else {
        msgErr("添加失败（节点不存在或路段已存在）");
    }
}

void MainWindow::onDeleteEdge() {
    QDialog dlg(this);
    dlg.setWindowTitle("删除路段");
    auto* form = new QFormLayout(&dlg);
    auto* fromSpin = new QSpinBox; fromSpin->setRange(1, 9999);
    auto* toSpin   = new QSpinBox; toSpin->setRange(1, 9999);
    form->addRow("起点编号：", fromSpin);
    form->addRow("终点编号：", toSpin);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    int from = fromSpin->value(), to = toSpin->value();
    if (_graph.deleteEdge(from, to)) {
        msgOk(QString("路段 %1→%2 已删除").arg(from).arg(to));
        clearHL(); updateStats(); refreshCanvas();
    } else {
        msgErr("路段不存在");
    }
}

void MainWindow::onListEdges() {
    msg("── 所有路段（" + QString::number(_graph.edgeCount()) + "）──", "#5ab4ff");
    DynArray<int> ids = _graph.getAllNodeIds();
    for (int i = 0; i < ids.size() - 1; ++i)
        for (int j=i+1; j<ids.size(); ++j)
            if (ids[i]>ids[j]) { int t=ids[i]; ids[i]=ids[j]; ids[j]=t; }
    for (int i = 0; i < ids.size(); ++i) {
        const DynArray<Edge>& es = _graph.getNeighbors(ids[i]);
        for (int j = 0; j < es.size(); ++j) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1);
            oss << es[j].from << "→" << es[j].to
                << "  " << es[j].time << "h  " << es[j].cost << "元";
            msg(QString::fromStdString(oss.str()));
        }
    }
}

void MainWindow::onImportNetwork() {
    QString path = QFileDialog::getOpenFileName(this, "导入路网", "data", "文本文件 (*.txt)");
    if (path.isEmpty()) return;
    _graph.clear();
    clearHL();
    if (FileManager::loadNetwork(path.toStdString(), _graph)) {
        msgOk("导入成功：" + QString::number(_graph.nodeCount()) +
              " 网点 / " + QString::number(_graph.edgeCount()) + " 路段");
        updateStats(); refreshCanvas();
    } else {
        msgErr("导入失败");
    }
}

void MainWindow::onExportNetwork() {
    QString path = QFileDialog::getSaveFileName(this, "导出路网", "data/network.txt", "文本文件 (*.txt)");
    if (path.isEmpty()) return;
    if (FileManager::saveNetwork(path.toStdString(), _graph))
        msgOk("路网已保存：" + path);
    else
        msgErr("保存失败");
}

// ── 路径查询 ──────────────────────────────────────────────
void MainWindow::onShortestTime() {
    QDialog dlg(this);
    dlg.setWindowTitle("单源最短耗时（Dijkstra）");
    auto* form = new QFormLayout(&dlg);
    auto* srcSpin = new QSpinBox; srcSpin->setRange(1, 9999);
    form->addRow("起点网点编号：", srcSpin);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;

    int src = srcSpin->value();
    clearHL();
    auto results = Dijkstra::shortestTimeFrom(_graph, src);
    if (results.empty()) { msgErr("起点不存在"); return; }

    _srcHl = src;
    msg("── 从[" + QString::number(src) + "]出发的最短耗时 ──", "#5ac4ff");

    DynArray<int> ids = _graph.getAllNodeIds();
    for (int i = 0; i < ids.size(); ++i) {
        int id = ids[i];
        if (id == src) continue;
        PathResult* pr = results.find(id);
        if (!pr || !pr->reachable) continue;
        // 高亮所有路径的边
        for (int j = 0; j + 1 < pr->path.size(); ++j) {
            _hlEdgeSrc.push_back(pr->path[j]);
            _hlEdgeDst.push_back(pr->path[j + 1]);
        }
        _hlNodes.push_back(id);
        std::ostringstream oss; oss << std::fixed << std::setprecision(1);
        const Node* n = _graph.findNode(id);
        oss << "[" << id << "]" << (n ? n->name : "")
            << "  " << pr->totalTime << "h / " << pr->totalCost << "元";
        msg(QString::fromStdString(oss.str()), "#c8deb0");
    }
    refreshCanvas();
}

void MainWindow::onCheapestPath() {
    QDialog dlg(this);
    dlg.setWindowTitle("两点最低费用路径");
    auto* form = new QFormLayout(&dlg);
    auto* srcSpin = new QSpinBox; srcSpin->setRange(1, 9999);
    auto* dstSpin = new QSpinBox; dstSpin->setRange(1, 9999);
    form->addRow("起点编号：", srcSpin);
    form->addRow("终点编号：", dstSpin);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;

    int src = srcSpin->value(), dst = dstSpin->value();
    clearHL();
    PathResult r = Dijkstra::cheapestPath(_graph, src, dst);
    if (!r.reachable) { msgErr("不可达"); return; }

    _srcHl = src; _dstHl = dst;
    applyPath(r.path);

    std::ostringstream oss; oss << std::fixed << std::setprecision(1);
    oss << "路径: ";
    for (int i = 0; i < r.path.size(); ++i) {
        oss << r.path[i];
        if (i + 1 < r.path.size()) oss << "→";
    }
    msgOk(QString::fromStdString(oss.str()));
    oss.str(""); oss << "费用: " << r.totalCost << "元  耗时: " << r.totalTime << "h";
    msgOk(QString::fromStdString(oss.str()));
}

void MainWindow::onClearHL() {
    clearHL();
    msgOk("已清除高亮");
}

// ── 批次配送 ──────────────────────────────────────────────
void MainWindow::onAddOrder() {
    QDialog dlg(this);
    dlg.setWindowTitle("添加配送订单");
    auto* form = new QFormLayout(&dlg);
    auto* oidSpin  = new QSpinBox; oidSpin->setRange(1, 99999);
    auto* srcSpin  = new QSpinBox; srcSpin->setRange(1, 9999);
    auto* dstSpin  = new QSpinBox; dstSpin->setRange(1, 9999);
    auto* goodsEdit = new QLineEdit;
    auto* optCombo = new QComboBox;
    optCombo->addItem("最低费用"); optCombo->addItem("最短耗时");
    form->addRow("订单号：",      oidSpin);
    form->addRow("起点编号：",    srcSpin);
    form->addRow("终点编号：",    dstSpin);
    form->addRow("货物描述：",    goodsEdit);
    form->addRow("优化目标：",    optCombo);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    Order o{oidSpin->value(), srcSpin->value(), dstSpin->value(),
            goodsEdit->text().toStdString(), optCombo->currentIndex() == 1};
    if (_om.addOrder(o)) {
        msgOk("订单 " + QString::number(o.orderId) + " 添加成功");
        updateStats();
    } else {
        msgErr("订单号已存在");
    }
}

void MainWindow::onRemoveOrder() {
    QDialog dlg(this);
    dlg.setWindowTitle("删除订单");
    auto* form = new QFormLayout(&dlg);
    auto* oidSpin = new QSpinBox; oidSpin->setRange(1, 99999);
    form->addRow("订单号：", oidSpin);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);
    if (dlg.exec() != QDialog::Accepted) return;
    int oid = oidSpin->value();
    if (_om.removeOrder(oid)) {
        msgOk("订单 " + QString::number(oid) + " 已删除");
        updateStats();
    } else {
        msgErr("订单不存在");
    }
}

void MainWindow::onListOrders() {
    const DynArray<Order>& orders = _om.getOrders();
    msg("── 所有订单（" + QString::number(orders.size()) + "）──", "#5ab4ff");
    for (int i = 0; i < orders.size(); ++i) {
        const Order& o = orders[i];
        msg(QString("[%1] %2→%3  %4  %5")
            .arg(o.orderId).arg(o.srcNode).arg(o.dstNode)
            .arg(QString::fromStdString(o.goods))
            .arg(o.byTime ? "最短耗时" : "最低费用"));
    }
}

void MainWindow::onImportOrders() {
    QString path = QFileDialog::getOpenFileName(this, "导入订单", "data", "文本文件 (*.txt)");
    if (path.isEmpty()) return;
    if (FileManager::loadOrders(path.toStdString(), _om)) {
        msgOk("导入订单：" + QString::number(_om.getOrders().size()) + " 条");
        updateStats();
    } else {
        msgErr("导入失败");
    }
}

void MainWindow::onPlanAll() {
    clearHL();
    DynArray<DeliveryPlan> plans = _om.planAllOrders(_graph);
    msg("── 批量规划结果 ──", "#5ac4ff");
    for (int i = 0; i < plans.size(); ++i) {
        const DeliveryPlan& p = plans[i];
        if (!p.result.reachable) {
            msgErr("订单 " + QString::number(p.order.orderId) + " 不可达");
            continue;
        }
        std::ostringstream oss; oss << std::fixed << std::setprecision(1);
        oss << "[" << p.order.orderId << "] " << p.order.goods << ": ";
        for (int j = 0; j < p.result.path.size(); ++j) {
            oss << p.result.path[j];
            if (j + 1 < p.result.path.size()) oss << "→";
        }
        double val = p.order.byTime ? p.result.totalTime : p.result.totalCost;
        oss << "  " << val << (p.order.byTime ? "h" : "元");
        msgOk(QString::fromStdString(oss.str()));
        applyPath(p.result.path);
    }
    msgOk("规划完成，共 " + QString::number(plans.size()) + " 条");
}

void MainWindow::onTopoSort() {
    clearHL();
    TopoResult res = _om.planBatchSequence(_graph);
    if (res.hasCycle) {
        msgErr("检测到环路！涉及节点：");
        for (int i = 0; i < res.cycleNodes.size(); ++i) {
            int id = res.cycleNodes[i];
            _hlNodes.push_back(id);
            const Node* n = _graph.findNode(id);
            msg(QString("  %1（%2）").arg(id)
                .arg(n ? QString::fromStdString(n->name) : "?"), "#e09050");
        }
    } else {
        msg("── 拓扑排序配送顺序 ──", "#5ac4ff");
        QString line;
        for (int i = 0; i < res.order.size(); ++i) {
            int id = res.order[i];
            _hlNodes.push_back(id);
            const Node* n = _graph.findNode(id);
            QString nm = n ? QString::fromStdString(n->name) : "?";
            if (nm.length() > 3) nm = nm.left(3);
            line += QString("%1(%2)").arg(id).arg(nm);
            if (i + 1 < res.order.size()) line += "→";
            if (line.length() > 48) { msg(line, "#c8d8b0"); line.clear(); }
        }
        if (!line.isEmpty()) msg(line, "#c8d8b0");
        msgOk(QString::number(res.order.size()) + " 个节点，无环路");
    }
    refreshCanvas();
}

void MainWindow::onExportPlans() {
    QString path = QFileDialog::getSaveFileName(this, "导出配送方案", "data/plans.txt", "文本文件 (*.txt)");
    if (path.isEmpty()) return;
    DynArray<DeliveryPlan> plans = _om.planAllOrders(_graph);
    if (FileManager::savePlans(path.toStdString(), _graph, plans))
        msgOk("方案已导出：" + path);
    else
        msgErr("导出失败");
}
