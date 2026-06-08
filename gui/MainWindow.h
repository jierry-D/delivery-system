#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QLabel>
#include <QStackedWidget>
#include <QPushButton>
#include "../include/Graph.h"
#include "../include/OrderManager.h"
#include "../include/DynArray.h"
#include "GraphWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    // 导航
    void goMain();
    void goNode();
    void goNetwork();
    void goPath();
    void goDelivery();

    // 网点管理
    void onAddNode();
    void onDeleteNode();
    void onUpdateNode();
    void onFindNode();
    void onListNodes();

    // 路网管理
    void onAddEdge();
    void onDeleteEdge();
    void onListEdges();
    void onImportNetwork();
    void onExportNetwork();

    // 路径查询
    void onShortestTime();
    void onCheapestPath();
    void onClearHL();

    // 批次配送
    void onAddOrder();
    void onRemoveOrder();
    void onListOrders();
    void onImportOrders();
    void onPlanAll();
    void onTopoSort();
    void onExportPlans();

private:
    Graph        _graph;
    OrderManager _om;

    // 高亮状态
    DynArray<int> _hlNodes, _hlEdgeSrc, _hlEdgeDst;
    int _srcHl = -1, _dstHl = -1;

    // UI
    QStackedWidget* _stack   = nullptr;
    QLabel*         _stats   = nullptr;
    QTextEdit*      _log     = nullptr;
    GraphWidget*    _canvas  = nullptr;
    QLabel*         _modeLabel = nullptr;

    void setupUi();
    QWidget* makeMainPage();
    QWidget* makeNodePage();
    QWidget* makeNetworkPage();
    QWidget* makePathPage();
    QWidget* makeDeliveryPage();

    QPushButton* makeBtn(const QString& text, bool isBack = false);

    void updateStats();
    void refreshCanvas();

    void msg(const QString& text, const QString& color = "#dcdcdc");
    void msgOk (const QString& text);
    void msgErr(const QString& text);

    void applyPath(const DynArray<int>& path);
    void clearHL();
};
