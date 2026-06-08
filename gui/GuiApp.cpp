#include "GuiApp.h"
#include "../include/Logger.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>

using namespace std;

// ══════════════════════════════════════════════════════════
//  构造 & 初始化
// ══════════════════════════════════════════════════════════

GuiApp::GuiApp()
    : window(sf::VideoMode((unsigned)WIN_W, (unsigned)WIN_H),
             "快递网点配送路径规划系统",
             sf::Style::Default)
{
    window.setFramerateLimit(60);

    vector<string> fonts = {
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc"
    };
    for (auto& p : fonts)
        if (font.loadFromFile(p)) { break; }

    initNodePositions();
    buildButtons();
    loadDefault();
}

void GuiApp::initNodePositions() {
    map<int, pair<float,float>> geo = {
        { 1,{116.4f,39.9f}},{ 2,{117.2f,39.1f}},{ 3,{114.5f,38.0f}},
        { 4,{112.5f,37.9f}},{ 5,{111.6f,40.8f}},{ 6,{123.4f,41.8f}},
        { 7,{121.6f,38.9f}},{ 8,{125.3f,43.9f}},{ 9,{126.5f,45.8f}},
        {10,{121.5f,31.2f}},{11,{118.8f,32.1f}},{12,{120.2f,30.3f}},
        {13,{117.3f,31.8f}},{14,{119.3f,26.1f}},{15,{115.9f,28.7f}},
        {16,{117.0f,36.7f}},{17,{113.6f,34.7f}},{18,{114.3f,30.6f}},
        {19,{112.9f,28.2f}},{20,{113.3f,23.1f}},{21,{114.1f,22.5f}},
        {22,{108.4f,22.8f}},{23,{104.1f,30.7f}},{24,{106.5f,29.6f}},
        {25,{108.9f,34.3f}}
    };
    float cx0 = PANEL_W+10, cx1 = WIN_W-10;
    float cy0 = 50, cy1 = WIN_H-20;
    float lonMin=103.5f, lonMax=128.0f, latMin=21.5f, latMax=46.5f;
    for (auto& [id,ll] : geo) {
        nodePos[id] = {
            cx0 + (ll.first  - lonMin)/(lonMax-lonMin)*(cx1-cx0),
            cy1 - (ll.second - latMin)/(latMax-latMin)*(cy1-cy0)
        };
    }
}

void GuiApp::loadDefault() {
    if (FileManager::loadNetwork("data/network.txt", graph))
        msgOk("路网加载：" + to_string(graph.nodeCount()) +
              " 网点 / " + to_string(graph.edgeCount()) + " 路段");
}

// ══════════════════════════════════════════════════════════
//  按钮构建
// ══════════════════════════════════════════════════════════

static Btn makeBtn(float x, float y, float w, float h,
                   const string& label, sf::Font& font, sf::Color col)
{
    Btn b;
    b.baseColor = col;
    b.bg.setPosition(x, y);
    b.bg.setSize({w, h});
    b.bg.setFillColor(col);
    b.bg.setOutlineColor({255,255,255,30});
    b.bg.setOutlineThickness(1.f);

    b.label.setFont(font);
    b.label.setString(sf::String::fromUtf8(label.begin(), label.end()));
    b.label.setCharacterSize(15);
    b.label.setFillColor(sf::Color::White);
    auto bounds = b.label.getLocalBounds();
    b.label.setPosition(x + (w-bounds.width)/2.f - bounds.left,
                        y + (h-bounds.height)/2.f - bounds.top - 2.f);
    return b;
}

void GuiApp::buildButtons() {
    buttons.clear();
    float bx=10.f, bw=PANEL_W-20.f, bh=42.f, by=155.f, gap=8.f;

    auto add = [&](const string& label, bool back=false) {
        buttons.push_back(makeBtn(bx, by, bw, bh, label,
                                  font, back ? cBtnBack : cBtn));
        by += bh + gap;
    };
    switch (mode) {
    case AppMode::MAIN:
        add("网点管理"); add("路网管理"); add("路径查询"); add("批次配送"); break;
    case AppMode::NODE:
        add("添加网点"); add("删除网点"); add("修改网点");
        add("查询网点"); add("显示所有网点"); add("返回", true); break;
    case AppMode::NETWORK:
        add("添加路段"); add("删除路段"); add("显示所有路段");
        add("导入路网文件"); add("导出路网文件"); add("返回", true); break;
    case AppMode::PATH:
        add("单源最短耗时路径"); add("两点最低费用路径");
        add("清除高亮"); add("返回", true); break;
    case AppMode::DELIVERY:
        add("添加订单"); add("删除订单"); add("显示订单");
        add("导入订单文件"); add("规划所有订单");
        add("批次拓扑排序"); add("导出配送方案"); add("返回", true); break;
    }
}

// ══════════════════════════════════════════════════════════
//  主循环
// ══════════════════════════════════════════════════════════

void GuiApp::run() {
    while (window.isOpen()) { handleEvents(); render(); }
}

void GuiApp::handleEvents() {
    sf::Event ev;
    while (window.pollEvent(ev)) {
        if (ev.type == sf::Event::Closed) window.close();

        // 鼠标移动 — 悬停
        if (ev.type == sf::Event::MouseMoved) {
            sf::Vector2i mp{ev.mouseMove.x, ev.mouseMove.y};
            hoveredNode = nodeAtPos(mp);
            for (auto& b : buttons) {
                bool nowHov = b.contains(mp);
                if (nowHov != b.hovered) {
                    b.hovered = nowHov;
                    sf::Color c = b.baseColor;
                    if (b.hovered) {
                        c.r = (uint8_t)min(255, (int)c.r + 25);
                        c.g = (uint8_t)min(255, (int)c.g + 25);
                        c.b = (uint8_t)min(255, (int)c.b + 25);
                    }
                    b.bg.setFillColor(c);
                }
            }
        }

        // 鼠标点击
        if (ev.type == sf::Event::MouseButtonPressed &&
            ev.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2i mp{ev.mouseButton.x, ev.mouseButton.y};
            for (size_t i = 0; i < buttons.size(); ++i)
                if (buttons[i].contains(mp)) { onButtonClick(i); break; }
        }

        // 输入框事件
        if (collectingInput) {
            if (ev.type == sf::Event::TextEntered) {
                uint32_t ch = ev.text.unicode;
                if (ch == 8 && !inputBuf.empty()) {
                    inputBuf.pop_back();
                    while (!inputBuf.empty() && (inputBuf.back()&0xC0)==0x80)
                        inputBuf.pop_back();
                } else if (ch >= 32 && ch != 127) {
                    if      (ch < 0x80)  { inputBuf += (char)ch; }
                    else if (ch < 0x800) {
                        inputBuf += (char)(0xC0|(ch>>6));
                        inputBuf += (char)(0x80|(ch&0x3F));
                    } else {
                        inputBuf += (char)(0xE0|(ch>>12));
                        inputBuf += (char)(0x80|((ch>>6)&0x3F));
                        inputBuf += (char)(0x80|(ch&0x3F));
                    }
                }
            }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Enter)  submitInput();
                if (ev.key.code == sf::Keyboard::Escape) cancelInput();
            }
        }
    }
}

// ══════════════════════════════════════════════════════════
//  渲染
// ══════════════════════════════════════════════════════════

void GuiApp::render() {
    window.clear(cCanvas);
    renderCanvas();
    renderPanel();
    if (collectingInput) renderInputOverlay();
    renderHoverInfo();
    window.display();
}

void GuiApp::renderCanvas() {
    sf::RectangleShape bg({WIN_W-PANEL_W, WIN_H});
    bg.setPosition(PANEL_W, 0);
    bg.setFillColor(cCanvas);
    window.draw(bg);

    if (graph.nodeCount() == 0) return;

    auto ids = graph.getAllNodeIds();

    // 画边
    for (int from : ids) {
        for (const Edge& e : graph.getNeighbors(from)) {
            if (!nodePos.count(from)||!nodePos.count(e.to)) continue;
            bool hl = hlEdges.count({from,e.to})>0;
            bool bidir = false;
            for (const Edge& re : graph.getNeighbors(e.to))
                if (re.to==from) { bidir=true; break; }
            drawEdge(nodePos[from], nodePos[e.to],
                     hl ? cEdgeHl : cEdge,
                     hl ? 3.5f : 1.8f, bidir);
        }
    }

    // 画节点
    for (int id : ids) {
        if (!nodePos.count(id)) continue;
        sf::Color col = cNodeDef;
        if      (id==srcHl)         col = cNodeSrc;
        else if (id==dstHl)         col = cNodeDst;
        else if (hlNodes.count(id)) col = cNodePath;
        drawNode(id, nodePos[id], col);
    }
}

void GuiApp::renderPanel() {
    // 面板背景
    sf::RectangleShape panel({PANEL_W, WIN_H});
    panel.setFillColor(cPanel);
    window.draw(panel);

    // 标题栏
    sf::RectangleShape titleBg({PANEL_W, 50.f});
    titleBg.setFillColor({15,28,42});
    window.draw(titleBg);
    window.draw(makeText("快递路径规划系统", 10.f, 12.f, 16, {90,190,255}));

    // 统计
    sf::RectangleShape statBg({PANEL_W, 98.f});
    statBg.setPosition(0, 50);
    statBg.setFillColor({22,38,52});
    window.draw(statBg);

    auto statLine = [&](const string& k, const string& v, float y){
        window.draw(makeText(k, 12.f, y, 13, {130,170,210}));
        window.draw(makeText(v, PANEL_W*0.55f, y, 13, {90,215,145}));
    };
    statLine("网点数量:", to_string(graph.nodeCount()),  58.f);
    statLine("路段数量:", to_string(graph.edgeCount()),  76.f);
    statLine("订单数量:", to_string(om.getOrders().size()), 94.f);

    // 模式标签
    string modeStr;
    switch(mode){
    case AppMode::MAIN:     modeStr="主菜单";break;
    case AppMode::NODE:     modeStr="网点管理";break;
    case AppMode::NETWORK:  modeStr="路网管理";break;
    case AppMode::PATH:     modeStr="路径查询";break;
    case AppMode::DELIVERY: modeStr="批次配送";break;
    }
    sf::RectangleShape modeBg({PANEL_W,22.f});
    modeBg.setPosition(0,130.f);
    modeBg.setFillColor({12,24,36});
    window.draw(modeBg);
    window.draw(makeText("▶ "+modeStr, 12.f, 133.f, 13, {160,200,240}));

    // 按钮
    for (const auto& b : buttons) b.draw(window);

    // 消息区
    float msgY = 155.f + (float)buttons.size() * 50.f;
    if (msgY < WIN_H - 10.f) {
        sf::RectangleShape msgBg({PANEL_W, WIN_H-msgY});
        msgBg.setPosition(0, msgY);
        msgBg.setFillColor({15,26,38});
        window.draw(msgBg);
        window.draw(makeText("─── 消息 ───", 8.f, msgY+3.f, 11, {50,70,90}));
        float ly = msgY + 17.f;
        int start = max(0, (int)messages.size()-MAX_MSG);
        for (int i=start; i<(int)messages.size() && ly<WIN_H-3.f; ++i) {
            window.draw(makeText(messages[i].text, 8.f, ly, 12, messages[i].color));
            ly += 15.f;
        }
    }
}

void GuiApp::renderInputOverlay() {
    // 遮罩
    sf::RectangleShape mask({WIN_W,WIN_H});
    mask.setFillColor({0,0,0,150});
    window.draw(mask);

    float dw=520.f, dh=165.f;
    float dx=(WIN_W-dw)/2.f, dy=(WIN_H-dh)/2.f;

    sf::RectangleShape dlg({dw,dh});
    dlg.setPosition(dx,dy);
    dlg.setFillColor({22,38,58});
    dlg.setOutlineColor({52,152,219});
    dlg.setOutlineThickness(2.f);
    window.draw(dlg);

    window.draw(makeText(inputTitle,   dx+14.f, dy+12.f, 15, {90,195,255}));
    window.draw(makeText(inputPrompt,  dx+14.f, dy+46.f, 14, {195,215,235}));

    // 进度
    if (inputQueue.size()>1) {
        string pg = to_string(inputIdx+1)+"/"+to_string(inputQueue.size());
        window.draw(makeText(pg, dx+dw-48.f, dy+12.f, 12, {70,110,150}));
    }

    // 输入框
    sf::RectangleShape ib({dw-28.f,34.f});
    ib.setPosition(dx+14.f, dy+68.f);
    ib.setFillColor({12,22,36});
    ib.setOutlineColor({70,130,190});
    ib.setOutlineThickness(1.5f);
    window.draw(ib);
    window.draw(makeText(inputBuf+"|", dx+20.f, dy+73.f, 14, {215,235,255}));

    window.draw(makeText("Enter 确认   Esc 取消", dx+14.f, dy+134.f, 12, {70,100,130}));
}

void GuiApp::renderHoverInfo() {
    if (hoveredNode<0 || !nodePos.count(hoveredNode)) return;
    const Node* n = graph.findNode(hoveredNode);
    if (!n) return;

    sf::Vector2f pos = nodePos[hoveredNode];
    float bx = pos.x+NODE_R+6.f, by = pos.y-30.f;
    if (bx+205.f > WIN_W) bx = pos.x-210.f;
    if (by < 5.f) by = 5.f;

    sf::RectangleShape tip({205.f,60.f});
    tip.setPosition(bx,by);
    tip.setFillColor({18,32,50,230});
    tip.setOutlineColor({70,130,190});
    tip.setOutlineThickness(1.f);
    window.draw(tip);

    window.draw(makeText("ID:"+to_string(n->id)+" "+n->name,
                         bx+6.f, by+5.f,  13, {90,200,255}));
    window.draw(makeText(n->address,
                         bx+6.f, by+22.f, 12, {175,195,215}));
    window.draw(makeText("出边: "+to_string(graph.getNeighbors(n->id).size()),
                         bx+6.f, by+38.f, 12, {140,180,210}));
}

// ══════════════════════════════════════════════════════════
//  绘图基元
// ══════════════════════════════════════════════════════════

void GuiApp::drawEdge(sf::Vector2f a, sf::Vector2f b,
                      sf::Color col, float thick, bool bidir)
{
    sf::Vector2f d=b-a;
    float len=sqrt(d.x*d.x+d.y*d.y);
    if (len<1.f) return;
    sf::Vector2f u=d/len, perp{-u.y,u.x};
    float off = bidir ? 4.5f : 0.f;
    sf::Vector2f p1=a+u*NODE_R+perp*off, p2=b-u*NODE_R+perp*off;
    sf::Vector2f pd=p2-p1;
    float plen=sqrt(pd.x*pd.x+pd.y*pd.y);
    if (plen<1.f) return;
    sf::RectangleShape line({plen,thick});
    line.setOrigin(0.f, thick/2.f);
    line.setPosition(p1);
    line.setRotation(atan2(pd.y,pd.x)*180.f/3.14159f);
    line.setFillColor(col);
    window.draw(line);
    drawArrowHead(p2, u, col);
}

void GuiApp::drawArrowHead(sf::Vector2f tip, sf::Vector2f dir,
                            sf::Color col, float sz)
{
    sf::Vector2f perp{-dir.y,dir.x};
    sf::ConvexShape arrow;
    arrow.setPointCount(3);
    arrow.setPoint(0, tip);
    arrow.setPoint(1, tip-dir*sz+perp*(sz*0.42f));
    arrow.setPoint(2, tip-dir*sz-perp*(sz*0.42f));
    arrow.setFillColor(col);
    window.draw(arrow);
}

void GuiApp::drawNode(int id, sf::Vector2f pos, sf::Color col) {
    // 阴影
    sf::CircleShape shadow(NODE_R+2.f);
    shadow.setOrigin(NODE_R+2.f,NODE_R+2.f);
    shadow.setPosition(pos.x+2.f,pos.y+2.f);
    shadow.setFillColor({0,0,0,55});
    window.draw(shadow);

    // 主圆
    sf::CircleShape circle(NODE_R);
    circle.setOrigin(NODE_R,NODE_R);
    circle.setPosition(pos);
    circle.setFillColor(col);
    circle.setOutlineColor(sf::Color::White);
    circle.setOutlineThickness(1.5f);
    window.draw(circle);

    // ID
    sf::Text idT;
    idT.setFont(font);
    idT.setString(to_string(id));
    idT.setCharacterSize(13);
    idT.setFillColor(sf::Color::White);
    idT.setStyle(sf::Text::Bold);
    auto b=idT.getLocalBounds();
    idT.setOrigin(b.left+b.width/2.f, b.top+b.height/2.f);
    idT.setPosition(pos);
    window.draw(idT);

    // 名称（截取前 3 汉字）
    const Node* n = graph.findNode(id);
    if (n && !n->name.empty()) {
        string sn = n->name;
        size_t cut=0, cnt=0;
        while (cut<sn.size() && cnt<3) {
            unsigned char c=sn[cut];
            cut += (c<0x80)?1:(c<0xE0)?2:(c<0xF0)?3:4;
            ++cnt;
        }
        sn = sn.substr(0, cut);
        sf::Text nt;
        nt.setFont(font);
        nt.setString(sf::String::fromUtf8(sn.begin(),sn.end()));
        nt.setCharacterSize(11);
        nt.setFillColor({45,55,70});
        auto nb=nt.getLocalBounds();
        nt.setOrigin(nb.left+nb.width/2.f, 0.f);
        nt.setPosition(pos.x, pos.y+NODE_R+2.f);
        window.draw(nt);
    }
}

sf::Text GuiApp::makeText(const string& s, float x, float y,
                           unsigned sz, sf::Color col)
{
    sf::Text t;
    t.setFont(font);
    t.setString(sf::String::fromUtf8(s.begin(),s.end()));
    t.setCharacterSize(sz);
    t.setFillColor(col);
    t.setPosition(x,y);
    return t;
}

// ══════════════════════════════════════════════════════════
//  输入系统
// ══════════════════════════════════════════════════════════

void GuiApp::startInput(const string& title, vector<InputStep> steps) {
    if (steps.empty()) return;
    inputTitle = title;
    inputQueue = move(steps);
    inputIdx   = 0;
    inputPrompt = inputQueue[0].prompt;
    inputBuf.clear();
    collectingInput = true;
}

void GuiApp::submitInput() {
    if (!collectingInput || inputIdx >= inputQueue.size()) return;
    inputQueue[inputIdx].handler(inputBuf);
    inputBuf.clear();
    ++inputIdx;
    if (inputIdx < inputQueue.size()) {
        inputPrompt = inputQueue[inputIdx].prompt;
    } else {
        collectingInput = false;
        inputQueue.clear();
        buildButtons();
    }
}

void GuiApp::cancelInput() {
    collectingInput = false;
    inputBuf.clear();
    inputQueue.clear();
    inputIdx = 0;
    msg("已取消");
}

// ══════════════════════════════════════════════════════════
//  消息 & 高亮
// ══════════════════════════════════════════════════════════

void GuiApp::msg(const string& s, sf::Color col) {
    messages.push_back({s, col});
    if (messages.size() > 200) messages.erase(messages.begin());
}
void GuiApp::msgOk (const string& s){ msg(s, {90,215,130}); }
void GuiApp::msgErr(const string& s){ msg(s, {225,95,75});  }

void GuiApp::applyPath(const vector<int>& path) {
    hlEdges.clear(); hlNodes.clear();
    for (size_t i=0; i+1<path.size(); ++i) {
        hlEdges.insert({path[i],path[i+1]});
        hlNodes.insert(path[i]);
        hlNodes.insert(path[i+1]);
    }
}
void GuiApp::clearHighlight() {
    hlEdges.clear(); hlNodes.clear();
    srcHl=-1; dstHl=-1;
}

// ══════════════════════════════════════════════════════════
//  按钮分发
// ══════════════════════════════════════════════════════════

void GuiApp::onButtonClick(size_t idx) {
    auto goBack = [this]{ mode=AppMode::MAIN; buildButtons(); };
    switch (mode) {
    case AppMode::MAIN:
        if (idx==0){mode=AppMode::NODE;     buildButtons();}
        else if(idx==1){mode=AppMode::NETWORK; buildButtons();}
        else if(idx==2){mode=AppMode::PATH;    buildButtons();}
        else if(idx==3){mode=AppMode::DELIVERY;buildButtons();}
        break;
    case AppMode::NODE:
        switch(idx){
        case 0:doAddNode();   break; case 1:doDeleteNode(); break;
        case 2:doUpdateNode();break; case 3:doFindNode();   break;
        case 4:doListNodes(); break; case 5:goBack();       break;
        } break;
    case AppMode::NETWORK:
        switch(idx){
        case 0:doAddEdge();      break; case 1:doDeleteEdge(); break;
        case 2:doListEdges();    break; case 3:doImportNetwork();break;
        case 4:doExportNetwork();break; case 5:goBack();        break;
        } break;
    case AppMode::PATH:
        switch(idx){
        case 0:doShortestTime();break;
        case 1:doCheapestPath();break;
        case 2:clearHighlight(); msgOk("已清除高亮"); break;
        case 3:goBack(); break;
        } break;
    case AppMode::DELIVERY:
        switch(idx){
        case 0:doAddOrder();    break; case 1:doRemoveOrder();break;
        case 2:doListOrders();  break; case 3:doImportOrders();break;
        case 4:doPlanAll();     break; case 5:doTopoSort();   break;
        case 6:doExportPlans(); break; case 7:goBack();       break;
        } break;
    }
}

int GuiApp::nodeAtPos(sf::Vector2i p) const {
    for (auto& [id,pos]:nodePos) {
        float dx=p.x-pos.x, dy=p.y-pos.y;
        if (sqrt(dx*dx+dy*dy)<=NODE_R+4.f) return id;
    }
    return -1;
}

// ══════════════════════════════════════════════════════════
//  网点操作（共享状态用 shared_ptr 避免悬空引用）
// ══════════════════════════════════════════════════════════

void GuiApp::doAddNode() {
    struct S { int id=0; string name,addr; };
    auto s = make_shared<S>();
    startInput("添加网点", {
        {"网点编号（整数）:", [s,this](const string& v){
            try { s->id=stoi(v); }
            catch(...){ msgErr("编号无效"); cancelInput(); }
        }},
        {"网点名称:", [s](const string& v){ s->name=v; }},
        {"网点地址:", [s,this](const string& v){
            s->addr=v;
            if (graph.addNode(Node(s->id,s->name,s->addr)))
                msgOk("网点 "+to_string(s->id)+" 添加成功");
            else msgErr("失败（编号已存在或名称为空）");
        }}
    });
}

void GuiApp::doDeleteNode() {
    startInput("删除网点", {
        {"要删除的网点编号:", [this](const string& v){
            try {
                int id=stoi(v);
                graph.deleteNode(id) ? msgOk("网点 "+v+" 已删除")
                                     : msgErr("网点不存在");
            } catch(...){ msgErr("编号无效"); }
        }}
    });
}

void GuiApp::doUpdateNode() {
    struct S { int id=0; string name; };
    auto s = make_shared<S>();
    startInput("修改网点", {
        {"要修改的网点编号:", [s,this](const string& v){
            try { s->id=stoi(v); }
            catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"新名称:", [s](const string& v){ s->name=v; }},
        {"新地址:", [s,this](const string& v){
            graph.updateNode(s->id,Node(s->id,s->name,v))
                ? msgOk("修改成功")
                : msgErr("修改失败（网点不存在）");
        }}
    });
}

void GuiApp::doFindNode() {
    startInput("查询网点", {
        {"网点编号:", [this](const string& v){
            try {
                int id=stoi(v);
                const Node* n=graph.findNode(id);
                if (n) {
                    msgOk("找到: ["+to_string(n->id)+"] "+n->name);
                    msg("  地址: "+n->address, {170,195,215});
                    hlNodes.clear(); hlNodes.insert(id);
                } else msgErr("网点不存在");
            } catch(...){ msgErr("无效"); }
        }}
    });
}

void GuiApp::doListNodes() {
    msg("── 所有网点("+to_string(graph.nodeCount())+") ──",{90,175,240});
    auto ids=graph.getAllNodeIds(); sort(ids.begin(),ids.end());
    for (int id:ids) {
        const Node* n=graph.findNode(id);
        if (n) msg("["+to_string(id)+"] "+n->name+" "+n->address,{175,195,215});
    }
}

// ── 路网操作 ─────────────────────────────────────────────

void GuiApp::doAddEdge() {
    struct S { int f=0,t=0; double ti=0; };
    auto s=make_shared<S>();
    startInput("添加路段", {
        {"起点网点编号:", [s,this](const string& v){
            try{ s->f=stoi(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"终点网点编号:", [s,this](const string& v){
            try{ s->t=stoi(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"耗时（小时，如 2.5）:", [s,this](const string& v){
            try{ s->ti=stod(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"费用（元，如 80）:", [s,this](const string& v){
            try {
                double co=stod(v);
                graph.addEdge(Edge(s->f,s->t,s->ti,co))
                    ? msgOk("路段 "+to_string(s->f)+"->"+to_string(s->t)+" 添加成功")
                    : msgErr("失败（节点不存在或路段已存在）");
            } catch(...){ msgErr("无效"); }
        }}
    });
}

void GuiApp::doDeleteEdge() {
    struct S { int f=0; };
    auto s=make_shared<S>();
    startInput("删除路段", {
        {"起点编号:", [s,this](const string& v){
            try{ s->f=stoi(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"终点编号:", [s,this](const string& v){
            try {
                int t=stoi(v);
                graph.deleteEdge(s->f,t)
                    ? msgOk("路段 "+to_string(s->f)+"->"+v+" 已删除")
                    : msgErr("路段不存在");
            } catch(...){ msgErr("无效"); }
        }}
    });
}

void GuiApp::doListEdges() {
    msg("── 所有路段("+to_string(graph.edgeCount())+") ──",{90,175,240});
    auto ids=graph.getAllNodeIds(); sort(ids.begin(),ids.end());
    for (int from:ids)
        for (const Edge& e:graph.getNeighbors(from)) {
            ostringstream oss;
            oss<<fixed<<setprecision(1)
               <<from<<"->"<<e.to
               <<"  "<<e.time<<"h  "<<e.cost<<"元";
            msg(oss.str(),{175,195,215});
        }
}

void GuiApp::doImportNetwork() {
    startInput("导入路网文件", {
        {"文件路径（回车=data/network.txt）:", [this](const string& v){
            string p=v.empty()?"data/network.txt":v;
            graph.clear();
            FileManager::loadNetwork(p,graph)
                ? msgOk("导入: "+to_string(graph.nodeCount())+"节点 "+to_string(graph.edgeCount())+"边")
                : msgErr("导入失败: "+p);
        }}
    });
}

void GuiApp::doExportNetwork() {
    startInput("导出路网文件", {
        {"保存路径（回车=data/network.txt）:", [this](const string& v){
            string p=v.empty()?"data/network.txt":v;
            FileManager::saveNetwork(p,graph)
                ? msgOk("已保存: "+p) : msgErr("保存失败");
        }}
    });
}

// ── 路径查询 ─────────────────────────────────────────────

void GuiApp::doShortestTime() {
    startInput("单源最短耗时（Dijkstra）", {
        {"起点网点编号:", [this](const string& v){
            try {
                int src=stoi(v);
                clearHighlight();
                auto res=Dijkstra::shortestTimeFrom(graph,src);
                if (res.empty()){ msgErr("起点不存在"); return; }
                srcHl=src;
                msg("── 从["+v+"]出发的最短耗时 ──",{90,195,255});
                auto ids=graph.getAllNodeIds(); sort(ids.begin(),ids.end());
                for (int id:ids) {
                    if (id==src||!res[id].reachable) continue;
                    for (size_t i=0;i+1<res[id].path.size();++i)
                        hlEdges.insert({res[id].path[i],res[id].path[i+1]});
                    hlNodes.insert(id);
                    ostringstream oss; oss<<fixed<<setprecision(1);
                    const Node* nd=graph.findNode(id);
                    oss<<"["<<id<<"]"<<(nd?nd->name:"")
                       <<" "<<res[id].totalTime<<"h/"<<res[id].totalCost<<"元";
                    msg(oss.str(),{195,215,175});
                }
            } catch(...){ msgErr("无效"); }
        }}
    });
}

void GuiApp::doCheapestPath() {
    struct S { int src=0; };
    auto s=make_shared<S>();
    startInput("两点最低费用路径", {
        {"起点网点编号:", [s,this](const string& v){
            try{ s->src=stoi(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"终点网点编号:", [s,this](const string& v){
            try {
                int dst=stoi(v);
                clearHighlight();
                auto r=Dijkstra::cheapestPath(graph,s->src,dst);
                if (!r.reachable){ msgErr("不可达"); return; }
                srcHl=s->src; dstHl=dst;
                applyPath(r.path);
                ostringstream oss; oss<<fixed<<setprecision(1);
                oss<<"路径: ";
                for (size_t i=0;i<r.path.size();++i){
                    oss<<r.path[i];
                    if (i+1<r.path.size()) oss<<"->";
                }
                msgOk(oss.str());
                oss.str("");
                oss<<"费用:"<<r.totalCost<<"元  耗时:"<<r.totalTime<<"h";
                msgOk(oss.str());
            } catch(...){ msgErr("无效"); }
        }}
    });
}

// ── 批次配送 ─────────────────────────────────────────────

void GuiApp::doAddOrder() {
    struct S { int oid=0,src=0,dst=0; string goods; };
    auto s=make_shared<S>();
    startInput("添加配送订单", {
        {"订单号:", [s,this](const string& v){
            try{ s->oid=stoi(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"起点编号:", [s,this](const string& v){
            try{ s->src=stoi(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"终点编号:", [s,this](const string& v){
            try{ s->dst=stoi(v); } catch(...){ msgErr("无效"); cancelInput(); }
        }},
        {"货物描述:", [s](const string& v){ s->goods=v; }},
        {"优化目标（0=费用 / 1=耗时）:", [s,this](const string& v){
            bool bt=(v!="0");
            om.addOrder(Order{s->oid,s->src,s->dst,s->goods,bt})
                ? msgOk("订单 "+to_string(s->oid)+" 添加成功")
                : msgErr("订单号已存在");
        }}
    });
}

void GuiApp::doRemoveOrder() {
    startInput("删除订单", {
        {"订单号:", [this](const string& v){
            try {
                om.removeOrder(stoi(v))
                    ? msgOk("订单 "+v+" 已删除")
                    : msgErr("订单不存在");
            } catch(...){ msgErr("无效"); }
        }}
    });
}

void GuiApp::doListOrders() {
    auto& orders=om.getOrders();
    msg("── 所有订单("+to_string(orders.size())+") ──",{90,175,240});
    for (const auto& o:orders)
        msg("["+to_string(o.orderId)+"] "+to_string(o.srcNode)+"->"+
            to_string(o.dstNode)+" "+o.goods+
            (o.byTime?" [时间]":" [费用]"), {175,195,215});
}

void GuiApp::doImportOrders() {
    startInput("导入订单文件", {
        {"文件路径（回车=data/orders.txt）:", [this](const string& v){
            string p=v.empty()?"data/orders.txt":v;
            FileManager::loadOrders(p,om)
                ? msgOk("导入: "+to_string(om.getOrders().size())+" 条订单")
                : msgErr("导入失败: "+p);
        }}
    });
}

void GuiApp::doPlanAll() {
    auto plans=om.planAllOrders(graph);
    msg("── 批量规划结果 ──",{90,195,255});
    clearHighlight();
    for (const auto& p:plans) {
        if (!p.result.reachable){ msgErr("订单"+to_string(p.order.orderId)+" 不可达"); continue; }
        ostringstream oss; oss<<fixed<<setprecision(1);
        oss<<"["<<p.order.orderId<<"] "<<p.order.goods<<": ";
        for (size_t i=0;i<p.result.path.size();++i){
            oss<<p.result.path[i];
            if (i+1<p.result.path.size()) oss<<"->";
        }
        oss<<" "<<(p.order.byTime?p.result.totalTime:p.result.totalCost)
           <<(p.order.byTime?"h":"元");
        msgOk(oss.str());
        applyPath(p.result.path);  // 高亮最后一条
    }
    msgOk("规划完成，共 "+to_string(plans.size())+" 条");
}

void GuiApp::doTopoSort() {
    auto res=om.planBatchSequence(graph);
    clearHighlight();
    if (res.hasCycle) {
        msgErr("检测到环路！涉及节点:");
        for (int id:res.cycleNodes) {
            hlNodes.insert(id);
            const Node* n=graph.findNode(id);
            msg("  "+to_string(id)+(n?" "+n->name:""),{225,145,75});
        }
    } else {
        msg("── 拓扑排序配送顺序 ──",{90,195,255});
        string line;
        for (size_t i=0;i<res.order.size();++i) {
            int id=res.order[i]; hlNodes.insert(id);
            const Node* n=graph.findNode(id);
            string nm=n?n->name:"?";
            if (nm.size()>6) nm=nm.substr(0,6);
            line+=to_string(id)+"("+nm+")";
            if (i+1<res.order.size()) line+="→";
            if (line.size()>48){ msg(line,{195,215,175}); line.clear(); }
        }
        if (!line.empty()) msg(line,{195,215,175});
        msgOk(to_string(res.order.size())+" 个节点，无环路");
    }
}

void GuiApp::doExportPlans() {
    startInput("导出配送方案", {
        {"保存路径（回车=data/plans.txt）:", [this](const string& v){
            string p=v.empty()?"data/plans.txt":v;
            auto plans=om.planAllOrders(graph);
            FileManager::savePlans(p,graph,plans)
                ? msgOk("已导出: "+p) : msgErr("导出失败");
        }}
    });
}
