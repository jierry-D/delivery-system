#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QStackedWidget>
#include <QPointF>
#include "../include/Graph.h"
#include "../include/OrderManager.h"

// ── 路网画布（内嵌于 MainWindow 头文件） ─────────────────
class GraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit GraphWidget(QWidget* parent=nullptr);
    void setGraph(const Graph* g);
    void setHL(const DynArray<int>& nodes, const DynArray<int>& esrc,
               const DynArray<int>& edst, int src, int dst);
    void clearHL();
signals:
    void nodeHovered(int id);
    void nodeClicked(int id);
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    const Graph* g_ = nullptr;
    HashMap<int,QPointF> pos_;
    DynArray<int> hlN_, hlSrc_, hlDst_;
    int srcHL_=-1, dstHL_=-1, hov_=-1;
    static constexpr float R=18.f;
    void updatePos();
    bool isNodeHL(int id) const;
    bool isEdgeHL(int f, int t) const;
    int  nodeAt(QPoint p) const;
    void drawNode(class QPainter&, int id, QPointF, QColor);
    void drawEdge(class QPainter&, QPointF a, QPointF b, QColor, float w, bool bi);
};

// ── 主窗口 ────────────────────────────────────────────────
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);
private slots:
    void goMain(); void goNode(); void goNetwork(); void goPath(); void goDelivery();
    void onAddNode();   void onDeleteNode(); void onUpdateNode();
    void onFindNode();  void onListNodes();
    void onAddEdge();   void onDeleteEdge(); void onListEdges();
    void onImportNet(); void onExportNet();
    void onShortTime(); void onCheapPath(); void onClearHL();
    void onAddOrder();  void onDelOrder();  void onListOrders();
    void onImportOrd(); void onPlanAll();   void onTopoSort();  void onExportPlans();
private:
    Graph        g_;
    OrderManager om_;
    DynArray<int> hlN_, hlSrc_, hlDst_;
    int srcHL_=-1, dstHL_=-1;

    QStackedWidget* stack_=nullptr;
    QLabel*         stats_=nullptr;
    QTextEdit*      log_=nullptr;
    GraphWidget*    canvas_=nullptr;
    QLabel*         mode_=nullptr;

    void buildUi();
    class QPushButton* btn(const QString& t, bool back=false);
    QWidget* makePage(std::initializer_list<std::pair<QString,void(MainWindow::*)()>> items);
    void updateStats(); void refresh();
    void msg(const QString& t, const QString& c="#dcdcdc");
    void msgOk(const QString& t); void msgErr(const QString& t);
    void applyPath(const DynArray<int>& path); void clearHL();
};
