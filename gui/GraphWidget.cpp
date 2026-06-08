#include "GraphWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>
#include <cmath>

static const QColor C_BG      {240, 245, 250};
static const QColor C_EDGE    {160, 180, 200, 180};
static const QColor C_EDGE_HL {230, 100, 30};
static const QColor C_NODE    { 74, 144, 217};
static const QColor C_SRC     { 39, 174,  96};
static const QColor C_DST     {231,  76,  60};
static const QColor C_PATH    {243, 156,  18};
static const QColor C_HOVER   {100, 180, 255};

// 地理坐标（经度, 纬度）
static const int GEO_N = 25;
static const struct { int id; float lon, lat; } GEO[GEO_N] = {
    { 1,116.4f,39.9f},{ 2,117.2f,39.1f},{ 3,114.5f,38.0f},
    { 4,112.5f,37.9f},{ 5,111.6f,40.8f},{ 6,123.4f,41.8f},
    { 7,121.6f,38.9f},{ 8,125.3f,43.9f},{ 9,126.5f,45.8f},
    {10,121.5f,31.2f},{11,118.8f,32.1f},{12,120.2f,30.3f},
    {13,117.3f,31.8f},{14,119.3f,26.1f},{15,115.9f,28.7f},
    {16,117.0f,36.7f},{17,113.6f,34.7f},{18,114.3f,30.6f},
    {19,112.9f,28.2f},{20,113.3f,23.1f},{21,114.1f,22.5f},
    {22,108.4f,22.8f},{23,104.1f,30.7f},{24,106.5f,29.6f},
    {25,108.9f,34.3f}
};
static const float LON_MIN=103.5f, LON_MAX=128.0f;
static const float LAT_MIN= 21.5f, LAT_MAX= 46.5f;

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(500, 400);
}

void GraphWidget::setGraph(const Graph* g) {
    _graph = g;
    updateNodePositions();
    update();
}

void GraphWidget::setHighlight(const DynArray<int>& nodes,
                                const DynArray<int>& esrc,
                                const DynArray<int>& edst,
                                int src, int dst)
{
    _hlNodes  = nodes;
    _hlEdgeSrc = esrc;
    _hlEdgeDst = edst;
    _srcHl = src;
    _dstHl = dst;
    update();
}

void GraphWidget::clearHighlight() {
    _hlNodes.clear();
    _hlEdgeSrc.clear();
    _hlEdgeDst.clear();
    _srcHl = _dstHl = -1;
    update();
}

void GraphWidget::resizeEvent(QResizeEvent*) {
    updateNodePositions();
}

void GraphWidget::updateNodePositions() {
    float W = (float)width()  - 20.f;
    float H = (float)height() - 20.f;
    if (W < 1 || H < 1) return;
    for (int i = 0; i < GEO_N; ++i) {
        float x = 10.f + (GEO[i].lon - LON_MIN) / (LON_MAX - LON_MIN) * W;
        float y = H + 10.f - (GEO[i].lat - LAT_MIN) / (LAT_MAX - LAT_MIN) * H;
        _nodePos.set(GEO[i].id, QPointF(x, y));
    }
}

bool GraphWidget::isNodeHL(int id) const {
    for (int i = 0; i < _hlNodes.size(); ++i)
        if (_hlNodes[i] == id) return true;
    return false;
}

bool GraphWidget::isEdgeHL(int from, int to) const {
    for (int i = 0; i < _hlEdgeSrc.size(); ++i)
        if (_hlEdgeSrc[i] == from && _hlEdgeDst[i] == to) return true;
    return false;
}

QPointF GraphWidget::nodeCenter(int id) const {
    const QPointF* p = _nodePos.find(id);
    return p ? *p : QPointF(-999, -999);
}

int GraphWidget::nodeAtPos(QPoint pt) const {
    for (int i = 0; i < GEO_N; ++i) {
        const QPointF* p = _nodePos.find(GEO[i].id);
        if (!p) continue;
        float dx = pt.x() - (float)p->x();
        float dy = pt.y() - (float)p->y();
        if (std::sqrt(dx*dx + dy*dy) <= NODE_R + 4.f) return GEO[i].id;
    }
    return -1;
}

// ── 绘图基元 ──────────────────────────────────────────────

void GraphWidget::drawArrow(QPainter& p, QPointF tip, QPointF dir, QColor col) {
    float len = std::sqrt((float)(dir.x()*dir.x() + dir.y()*dir.y()));
    if (len < 0.01f) return;
    QPointF u(dir.x()/len, dir.y()/len);
    QPointF perp(-u.y(), u.x());
    float sz = 10.f;
    QPointF a = tip - u*sz + perp*(sz*0.42f);
    QPointF b = tip - u*sz - perp*(sz*0.42f);
    QPolygonF tri;
    tri << tip << a << b;
    p.setPen(Qt::NoPen);
    p.setBrush(col);
    p.drawPolygon(tri);
}

void GraphWidget::drawEdge(QPainter& p, QPointF a, QPointF b,
                            QColor col, float thick, bool bidir)
{
    QPointF d = b - a;
    float len = std::sqrt((float)(d.x()*d.x() + d.y()*d.y()));
    if (len < 1.f) return;
    QPointF u(d.x()/len, d.y()/len);
    QPointF perp(-u.y(), u.x());
    float off = bidir ? 4.5f : 0.f;
    QPointF p1 = a + u*NODE_R + perp*off;
    QPointF p2 = b - u*NODE_R + perp*off;
    p.setPen(QPen(col, thick));
    p.drawLine(p1, p2);
    drawArrow(p, p2, u, col);
}

void GraphWidget::drawNode(QPainter& p, int id, QPointF pos, QColor col) {
    // 阴影
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 45));
    p.drawEllipse(pos + QPointF(2, 2), (double)NODE_R + 2, (double)NODE_R + 2);

    // 主圆
    p.setPen(QPen(Qt::white, 1.5));
    p.setBrush(col);
    p.drawEllipse(pos, (double)NODE_R, (double)NODE_R);

    // ID 文字
    QFont f = p.font();
    f.setPointSize(10);
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::white);
    p.drawText(QRectF(pos.x()-NODE_R, pos.y()-NODE_R, NODE_R*2, NODE_R*2),
               Qt::AlignCenter, QString::number(id));

    // 节点名（最多3个字）
    if (_graph) {
        const Node* n = _graph->findNode(id);
        if (n && !n->name.empty()) {
            QString qname = QString::fromStdString(n->name);
            if (qname.length() > 3) qname = qname.left(3);
            f.setPointSize(9);
            f.setBold(false);
            p.setFont(f);
            p.setPen(QColor(40, 55, 70));
            p.drawText(QRectF(pos.x()-30, pos.y()+NODE_R+1, 60, 16),
                       Qt::AlignCenter, qname);
        }
    }
}

// ── 主绘制 ────────────────────────────────────────────────

void GraphWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 背景
    p.fillRect(rect(), C_BG);

    if (!_graph || _graph->nodeCount() == 0) {
        p.setPen(QColor(150, 160, 170));
        p.drawText(rect(), Qt::AlignCenter, "（路网为空）");
        return;
    }

    DynArray<int> ids = _graph->getAllNodeIds();

    // 画边
    for (int i = 0; i < ids.size(); ++i) {
        int from = ids[i];
        const QPointF* pa = _nodePos.find(from);
        if (!pa) continue;
        const DynArray<Edge>& nbrs = _graph->getNeighbors(from);
        for (int j = 0; j < nbrs.size(); ++j) {
            int to = nbrs[j].to;
            const QPointF* pb = _nodePos.find(to);
            if (!pb) continue;
            bool hl = isEdgeHL(from, to);
            // 判断是否双向
            bool bidir = false;
            const DynArray<Edge>& rev = _graph->getNeighbors(to);
            for (int k = 0; k < rev.size(); ++k)
                if (rev[k].to == from) { bidir = true; break; }
            drawEdge(p, *pa, *pb,
                     hl ? C_EDGE_HL : C_EDGE,
                     hl ? 3.0f : 1.5f, bidir);
        }
    }

    // 画节点
    for (int i = 0; i < ids.size(); ++i) {
        int id = ids[i];
        const QPointF* pos = _nodePos.find(id);
        if (!pos) continue;
        QColor col = C_NODE;
        if      (id == _srcHl)    col = C_SRC;
        else if (id == _dstHl)    col = C_DST;
        else if (isNodeHL(id))    col = C_PATH;
        else if (id == _hoveredNode) col = C_HOVER;
        drawNode(p, id, *pos, col);
    }

    // 悬停提示
    if (_hoveredNode > 0 && _graph) {
        const Node* n = _graph->findNode(_hoveredNode);
        const QPointF* pos = _nodePos.find(_hoveredNode);
        if (n && pos) {
            QString tip = QString("[%1] %2\n%3\n出边: %4")
                .arg(n->id)
                .arg(QString::fromStdString(n->name))
                .arg(QString::fromStdString(n->address))
                .arg(_graph->getNeighbors(n->id).size());
            float bx = (float)pos->x() + NODE_R + 8;
            float by = (float)pos->y() - 35;
            if (bx + 200 > width())  bx = (float)pos->x() - 208;
            if (by < 5)              by = 5;
            QRectF box(bx, by, 200, 70);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(18, 32, 50, 220));
            p.drawRoundedRect(box, 6, 6);
            p.setPen(QColor(70, 130, 190));
            p.drawRoundedRect(box, 6, 6);
            QFont f = p.font(); f.setPointSize(9); p.setFont(f);
            p.setPen(QColor(90, 200, 255));
            p.drawText(box.adjusted(6, 5, -4, -45), Qt::AlignLeft,
                       QString("[%1] %2").arg(n->id).arg(QString::fromStdString(n->name)));
            p.setPen(QColor(175, 195, 215));
            p.drawText(box.adjusted(6, 25, -4, -25), Qt::AlignLeft,
                       QString::fromStdString(n->address));
            p.setPen(QColor(140, 180, 210));
            p.drawText(box.adjusted(6, 48, -4, -5), Qt::AlignLeft,
                       QString("出边: %1").arg(_graph->getNeighbors(n->id).size()));
        }
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent* ev) {
    int id = nodeAtPos(ev->pos());
    if (id != _hoveredNode) {
        _hoveredNode = id;
        emit nodeHovered(id);
        update();
    }
}

void GraphWidget::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() == Qt::LeftButton) {
        int id = nodeAtPos(ev->pos());
        if (id > 0) emit nodeClicked(id);
    }
}
