#pragma once

struct Edge {
    int from;
    int to;
    double time;  // 运输耗时（小时）
    double cost;  // 运输费用（元）

    Edge() : from(0), to(0), time(0), cost(0) {}
    Edge(int from, int to, double time, double cost)
        : from(from), to(to), time(time), cost(cost) {}
};
