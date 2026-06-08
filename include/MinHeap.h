#pragma once
#include "DynArray.h"

// 默认比较器（小值优先）
template<typename T>
struct HeapLess { bool operator()(const T& a, const T& b) const { return a < b; } };

// 自实现最小堆（替代 std::priority_queue<..., greater<>>）
// Compare(a, b) 返回 true 表示 a 优先级高于 b（即 a "更小"）
template<typename T, typename Compare = HeapLess<T>>
class MinHeap {
    DynArray<T> _data;
    Compare     _cmp;

    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (_cmp(_data[i], _data[p])) {
                T tmp = std::move(_data[i]);
                _data[i] = std::move(_data[p]);
                _data[p] = std::move(tmp);
                i = p;
            } else break;
        }
    }

    void siftDown(int i) {
        int n = _data.size();
        while (true) {
            int best = i;
            int l = 2 * i + 1, r = 2 * i + 2;
            if (l < n && _cmp(_data[l], _data[best])) best = l;
            if (r < n && _cmp(_data[r], _data[best])) best = r;
            if (best == i) break;
            T tmp = std::move(_data[i]);
            _data[i]    = std::move(_data[best]);
            _data[best] = std::move(tmp);
            i = best;
        }
    }

public:
    MinHeap() = default;
    explicit MinHeap(Compare cmp) : _cmp(cmp) {}

    bool      empty() const { return _data.empty(); }
    int       size()  const { return _data.size(); }
    const T&  top()   const { return _data[0]; }

    void push(const T& item) {
        _data.push_back(item);
        siftUp(_data.size() - 1);
    }

    void pop() {
        if (_data.empty()) return;
        _data[0] = std::move(_data.back());
        _data.pop_back();
        if (!_data.empty()) siftDown(0);
    }
};
