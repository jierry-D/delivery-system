#pragma once
#include <utility>
#include <stdexcept>

// 自实现动态数组（替代 std::vector）
template<typename T>
class DynArray {
    T*  _data;
    int _size;
    int _cap;

    void grow() {
        int nc = (_cap == 0) ? 4 : _cap * 2;
        T* nd = new T[nc];
        for (int i = 0; i < _size; ++i) nd[i] = std::move(_data[i]);
        delete[] _data;
        _data = nd;
        _cap  = nc;
    }

public:
    DynArray() : _data(nullptr), _size(0), _cap(0) {}

    DynArray(const DynArray& o) : _data(nullptr), _size(0), _cap(0) {
        reserve(o._size);
        for (int i = 0; i < o._size; ++i) push_back(o._data[i]);
    }

    DynArray(DynArray&& o) noexcept
        : _data(o._data), _size(o._size), _cap(o._cap) {
        o._data = nullptr; o._size = 0; o._cap = 0;
    }

    DynArray& operator=(const DynArray& o) {
        if (this != &o) {
            clear();
            reserve(o._size);
            for (int i = 0; i < o._size; ++i) push_back(o._data[i]);
        }
        return *this;
    }

    DynArray& operator=(DynArray&& o) noexcept {
        if (this != &o) {
            delete[] _data;
            _data = o._data; _size = o._size; _cap = o._cap;
            o._data = nullptr; o._size = 0; o._cap = 0;
        }
        return *this;
    }

    ~DynArray() { delete[] _data; }

    void push_back(const T& v) { if (_size == _cap) grow(); _data[_size++] = v; }
    void push_back(T&& v)      { if (_size == _cap) grow(); _data[_size++] = std::move(v); }
    void pop_back()            { if (_size > 0) --_size; }

    T&       operator[](int i)       { return _data[i]; }
    const T& operator[](int i) const { return _data[i]; }
    T&       back()                  { return _data[_size - 1]; }
    const T& back()            const { return _data[_size - 1]; }

    int  size()  const { return _size; }
    bool empty() const { return _size == 0; }

    void clear()        { _size = 0; }

    void reserve(int n) {
        if (n > _cap) {
            T* nd = new T[n];
            for (int i = 0; i < _size; ++i) nd[i] = std::move(_data[i]);
            delete[] _data;
            _data = nd; _cap = n;
        }
    }

    // 按下标删除（保序）
    void erase(int idx) {
        for (int i = idx; i < _size - 1; ++i)
            _data[i] = std::move(_data[i + 1]);
        --_size;
    }

    // 找到第一个满足谓词的元素并删除，返回是否删除成功
    template<typename Pred>
    bool remove_first_if(Pred pred) {
        for (int i = 0; i < _size; ++i) {
            if (pred(_data[i])) { erase(i); return true; }
        }
        return false;
    }

    // 删除所有满足谓词的元素
    template<typename Pred>
    int remove_all_if(Pred pred) {
        int cnt = 0;
        for (int i = 0; i < _size; ) {
            if (pred(_data[i])) { erase(i); ++cnt; }
            else ++i;
        }
        return cnt;
    }

    T* begin() { return _data; }
    T* end()   { return _data + _size; }
    const T* begin() const { return _data; }
    const T* end()   const { return _data + _size; }
};
