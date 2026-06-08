#pragma once
#include <QWidget>
#include <QPointF>
#include "../include/Graph.h"
#include "../include/DynArray.h"
#include "../include/HashMap.h"

class GraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit GraphWidget(QWidget* parent = nullptr);

    void setGraph(const Graph* g);

    // 设置高亮：hlNodes=途经节点, srcHl/dstHl=起终点, edges=高亮边
    void setHighlight(const DynArray<int>& nodes,
                      const DynArray<int>& edgeSrc,
                      const DynArray<int>& edgeDst,
                      int srcHl, int dstHl);
    void clearHighlight();

signals:
    void nodeClicked(int id);
    void nodeHovered(int id);  // -1 表示离开

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    const Graph* _graph = nullptr;

    // 地理坐标→窗口坐标映射（在 resizeEvent / setGraph 时更新）
    HashMap<int, QPointF> _nodePos;

    // 高亮状态
    DynArray<int> _hlNodes;
    DynArray<int> _hlEdgeSrc, _hlEdgeDst;
    int _srcHl = -1, _dstHl = -1;
    int _hoveredNode = -1;

    static constexpr float NODE_R = 18.f;

    void updateNodePositions();
    QPointF nodeCenter(int id) const;
    bool    isNodeHL(int id) const;
    bool    isEdgeHL(int from, int to) const;

    void drawEdge(class QPainter& p, QPointF a, QPointF b,
                  QColor col, float thick, bool bidir);
    void drawArrow(class QPainter& p, QPointF tip, QPointF dir, QColor col);
    void drawNode(class QPainter& p, int id, QPointF pos, QColor col);

    int nodeAtPos(QPoint pt) const;
};
