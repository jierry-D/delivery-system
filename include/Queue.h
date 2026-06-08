#pragma once
#include "DynArray.h"

// 自实现先进先出队列（替代 std::queue）
// 使用动态数组 + 头部偏移，push 均摊 O(1)，pop O(1)
template<typename T>
class Queue {
    DynArray<T> _buf;
    int         _head = 0;

public:
    Queue() = default;

    bool empty() const { return _head >= _buf.size(); }
    int  size()  const { return _buf.size() - _head; }

    void push(const T& v) { _buf.push_back(v); }
    void push(T&& v)      { _buf.push_back(std::move(v)); }

    T& front() { return _buf[_head]; }
    const T& front() const { return _buf[_head]; }

    void pop() { if (!empty()) ++_head; }

    void clear() { _buf.clear(); _head = 0; }
};
