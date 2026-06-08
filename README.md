# 快递网点配送路径规划系统

基于图论的快递网络路径规划系统，支持命令行与图形界面（SFML）双模式运行。数据结构课程大作业。

## 功能

- **网点管理**：增删改查快递网点（编号、名称、地址）
- **路网管理**：管理网点间的路段（耗时、费用），支持从文件导入/导出
- **路径查询**：
  - 单源最短耗时路径（Dijkstra，从一个网点到所有其他网点）
  - 两点最低费用路径（Dijkstra，按费用权重）
- **批次配送**：
  - 添加/删除/导入配送订单，每条订单可选最优化目标（最短时间或最低费用）
  - 批量规划所有订单路径
  - 拓扑排序生成批次配送顺序，并检测环路
  - 导出配送方案到文件

预置路网数据：**25 个全国主要城市仓库**，**61 条路段**（含耗时与费用权重）。

## 核心数据结构与算法

| 模块 | 实现 |
|------|------|
| 图 | 邻接表（`unordered_map` + `vector<Edge>`） |
| 最短路径 | Dijkstra 算法（优先队列实现） |
| 批次排序 | Kahn 算法拓扑排序，支持环路检测 |
| 持久化 | 自定义文本格式，FileManager 读写 |
| 日志 | 单例 Logger，写入 `logs/system.log` |

## 项目结构

```
delivery_system/
├── main.cpp          # CLI 入口
├── main_gui.cpp      # GUI 入口
├── Makefile
├── include/          # 头文件
│   ├── Node.h        # 网点结构体
│   ├── Edge.h        # 路段结构体
│   ├── Graph.h       # 图接口
│   ├── Dijkstra.h    # 最短路径
│   ├── TopoSort.h    # 拓扑排序
│   ├── OrderManager.h
│   ├── FileManager.h
│   └── Logger.h
├── src/              # 实现文件
├── gui/              # SFML 图形界面
│   ├── GuiApp.h
│   └── GuiApp.cpp
└── data/
    ├── network.txt   # 路网数据
    └── orders.txt    # 订单数据
```

## 编译与运行

**依赖**：g++（支持 C++17）、SFML 2.x（仅 GUI 模式需要）

```bash
# Ubuntu/Debian 安装 SFML
sudo apt install libsfml-dev

# 编译 CLI 版本
make cli

# 编译 GUI 版本
make gui

# 编译并运行 CLI
make run-cli

# 编译并运行 GUI
make run-gui

# 清理编译产物
make clean
```

## 数据文件格式

**network.txt**（路网）：
```
NODE <id> <名称> <地址>
EDGE <from> <to> <耗时h> <费用元>
```

**orders.txt**（订单）：
```
<订单号> <起点id> <终点id> <货物描述> <优化目标(0=费用/1=时间)>
```
