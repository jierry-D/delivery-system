#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <string>
#include <map>
#include <set>
#include "../include/Graph.h"
#include "../include/Dijkstra.h"
#include "../include/TopoSort.h"
#include "../include/OrderManager.h"
#include "../include/FileManager.h"

// ─── 简单按钮控件 ─────────────────────────────────────────
struct Btn {
    sf::RectangleShape bg;
    sf::Text           label;
    sf::Color          baseColor;
    bool               hovered = false;

    bool contains(sf::Vector2i p) const {
        return bg.getGlobalBounds().contains((float)p.x, (float)p.y);
    }
    void draw(sf::RenderWindow& w) const { w.draw(bg); w.draw(label); }
};

// ─── 输入步骤 ──────────────────────────────────────────────
struct InputStep {
    std::string prompt;
    std::function<void(const std::string&)> handler;
};

// ─── 消息行 ──────────────────────────────────────────────
struct MsgLine {
    std::string text;
    sf::Color   color;
};

enum class AppMode { MAIN, NODE, NETWORK, PATH, DELIVERY };

// ─── 主 GUI 类 ────────────────────────────────────────────
class GuiApp {
public:
    GuiApp();
    void run();

private:
    // 窗口 & 字体
    sf::RenderWindow window;
    sf::Font         font;

    // 数据层
    Graph        graph;
    OrderManager om;

    // 各节点在窗口上的坐标
    std::map<int, sf::Vector2f> nodePos;

    // 当前模式
    AppMode mode = AppMode::MAIN;

    // 高亮路径
    std::set<std::pair<int,int>> hlEdges;
    std::set<int>                hlNodes;
    int srcHl = -1, dstHl = -1;

    // 输入序列
    bool collectingInput = false;
    std::string inputTitle;
    std::string inputPrompt;
    std::string inputBuf;
    std::vector<InputStep> inputQueue;
    size_t inputIdx = 0;

    // 侧栏按钮
    std::vector<Btn> buttons;

    // 消息日志
    std::vector<MsgLine> messages;

    // 鼠标悬停的节点
    int hoveredNode = -1;

    // 常量
    static constexpr float PANEL_W   = 310.f;
    static constexpr float WIN_W     = 1340.f;
    static constexpr float WIN_H     = 780.f;
    static constexpr float NODE_R    = 18.f;
    static constexpr int   MAX_MSG   = 12;

    // ─ 颜色 ─
    sf::Color cPanel    {30,  45,  60};
    sf::Color cCanvas   {240, 245, 250};
    sf::Color cBtn      {52,  152, 219};
    sf::Color cBtnHov   {41,  128, 185};
    sf::Color cBtnBack  {100, 120, 135};
    sf::Color cNodeDef  {74,  144, 217};
    sf::Color cNodeSrc  {39,  174, 96};
    sf::Color cNodeDst  {231, 76,  60};
    sf::Color cNodePath {243, 156, 18};
    sf::Color cEdge     {170, 185, 200, 200};
    sf::Color cEdgeHl   {230, 100, 30};
    sf::Color cText     {255, 255, 255};
    sf::Color cTextDark {40,  55,  70};

    // ─ 初始化 ─
    void initNodePositions();
    void buildButtons();
    void loadDefault();

    // ─ 主循环 ─
    void handleEvents();
    void render();

    // ─ 渲染 ─
    void renderCanvas();
    void renderPanel();
    void renderInputOverlay();
    void renderHoverInfo();

    // ─ 绘图基元 ─
    void drawEdge(sf::Vector2f a, sf::Vector2f b,
                  sf::Color col, float thick = 2.f, bool bidirectional = false);
    void drawArrowHead(sf::Vector2f tip, sf::Vector2f dir,
                       sf::Color col, float sz = 10.f);
    void drawNode(int id, sf::Vector2f pos, sf::Color col);
    sf::Text makeText(const std::string& s, float x, float y,
                      unsigned sz = 14, sf::Color col = {255,255,255});

    // ─ 按钮点击分发 ─
    void onButtonClick(size_t idx);

    // ─ 输入系统 ─
    void startInput(const std::string& title,
                    std::vector<InputStep> steps);
    void submitInput();
    void cancelInput();

    // ─ 消息 ─
    void msg(const std::string& s, sf::Color col = {220,220,220});
    void msgOk (const std::string& s);
    void msgErr(const std::string& s);

    // ─ 高亮 ─
    void applyPath(const std::vector<int>& path);
    void clearHighlight();

    // ─ 画布点击 ─
    int nodeAtPos(sf::Vector2i p) const;

    // ─── 各功能入口 ───────────────────────────────────────
    void doAddNode();
    void doDeleteNode();
    void doUpdateNode();
    void doFindNode();
    void doListNodes();

    void doAddEdge();
    void doDeleteEdge();
    void doListEdges();
    void doImportNetwork();
    void doExportNetwork();

    void doShortestTime();
    void doCheapestPath();

    void doAddOrder();
    void doRemoveOrder();
    void doListOrders();
    void doImportOrders();
    void doPlanAll();
    void doTopoSort();
    void doExportPlans();
};
