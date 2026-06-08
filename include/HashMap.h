#pragma once
#include <utility>
#include <string>

// 自实现哈希表（开放地址法 + 线性探测，替代 std::map / std::unordered_map）
template<typename K, typename V>
class HashMap {
    struct Slot {
        K    key;
        V    value;
        bool used    = false;
        bool deleted = false;
    };

    Slot* _slots;
    int   _cap;
    int   _size;

    // ── 哈希函数 ─────────────────────────────────────
    static unsigned hashKey(int k, int cap) {
        unsigned h = (unsigned)k * 2654435761u;
        return h % (unsigned)cap;
    }
    static unsigned hashKey(const std::string& k, int cap) {
        unsigned h = 5381;
        for (char c : k) h = h * 33 + (unsigned char)c;
        return h % (unsigned)cap;
    }
    unsigned slot0(const K& k) const {
        if constexpr (std::is_same_v<K, int>)
            return hashKey((int)k, _cap);
        else if constexpr (std::is_same_v<K, std::string>)
            return hashKey(k, _cap);
        else
            return (unsigned)(std::hash<K>{}(k) % (unsigned)_cap);
    }

    // ── 扩容并重哈希 ──────────────────────────────────
    void rehash(int newCap) {
        Slot* old    = _slots;
        int   oldCap = _cap;
        _slots = new Slot[newCap]();
        _cap   = newCap;
        _size  = 0;
        for (int i = 0; i < oldCap; ++i)
            if (old[i].used && !old[i].deleted)
                set(old[i].key, std::move(old[i].value));
        delete[] old;
    }

    // ── 内部写入（不检查扩容，用于 rehash 内部）─────
    void insertRaw(const K& k, V&& v) {
        unsigned idx = slot0(k);
        while (_slots[idx].used && !_slots[idx].deleted && !(_slots[idx].key == k))
            idx = (idx + 1) % (unsigned)_cap;
        bool isNew = !_slots[idx].used || _slots[idx].deleted;
        _slots[idx].key     = k;
        _slots[idx].value   = std::move(v);
        _slots[idx].used    = true;
        _slots[idx].deleted = false;
        if (isNew) ++_size;
    }

public:
    explicit HashMap(int initCap = 16)
        : _slots(new Slot[initCap]()), _cap(initCap), _size(0) {}

    HashMap(const HashMap& o)
        : _slots(new Slot[o._cap]()), _cap(o._cap), _size(o._size) {
        for (int i = 0; i < _cap; ++i) _slots[i] = o._slots[i];
    }

    HashMap(HashMap&& o) noexcept
        : _slots(o._slots), _cap(o._cap), _size(o._size) {
        o._slots = nullptr; o._cap = 0; o._size = 0;
    }

    HashMap& operator=(const HashMap& o) {
        if (this != &o) {
            delete[] _slots;
            _cap = o._cap; _size = o._size;
            _slots = new Slot[_cap]();
            for (int i = 0; i < _cap; ++i) _slots[i] = o._slots[i];
        }
        return *this;
    }

    HashMap& operator=(HashMap&& o) noexcept {
        if (this != &o) {
            delete[] _slots;
            _slots = o._slots; _cap = o._cap; _size = o._size;
            o._slots = nullptr; o._cap = 0; o._size = 0;
        }
        return *this;
    }

    ~HashMap() { delete[] _slots; }

    // ── 写入 ─────────────────────────────────────────
    void set(const K& k, const V& v) {
        if (_size * 2 >= _cap) rehash(_cap * 2);
        unsigned idx = slot0(k);
        int firstDel = -1;
        while (_slots[idx].used) {
            if (_slots[idx].deleted) {
                if (firstDel < 0) firstDel = (int)idx;
            } else if (_slots[idx].key == k) {
                _slots[idx].value = v;
                return;
            }
            idx = (idx + 1) % (unsigned)_cap;
        }
        int ins = (firstDel >= 0) ? firstDel : (int)idx;
        _slots[ins].key     = k;
        _slots[ins].value   = v;
        _slots[ins].used    = true;
        _slots[ins].deleted = false;
        ++_size;
    }

    void set(const K& k, V&& v) {
        if (_size * 2 >= _cap) rehash(_cap * 2);
        insertRaw(k, std::move(v));
    }

    // ── 下标访问（不存在则默认构造）─────────────────
    V& operator[](const K& k) {
        if (_size * 2 >= _cap) rehash(_cap * 2);
        unsigned idx = slot0(k);
        int firstDel = -1;
        while (_slots[idx].used) {
            if (_slots[idx].deleted) {
                if (firstDel < 0) firstDel = (int)idx;
            } else if (_slots[idx].key == k) {
                return _slots[idx].value;
            }
            idx = (idx + 1) % (unsigned)_cap;
        }
        int ins = (firstDel >= 0) ? firstDel : (int)idx;
        _slots[ins].key     = k;
        _slots[ins].value   = V{};
        _slots[ins].used    = true;
        _slots[ins].deleted = false;
        ++_size;
        return _slots[ins].value;
    }

    // ── 查找（返回指针，nullptr 表示不存在）──────────
    V* find(const K& k) {
        unsigned idx = slot0(k);
        unsigned start = idx;
        bool wrapped = false;
        while (_slots[idx].used) {
            if (!_slots[idx].deleted && _slots[idx].key == k)
                return &_slots[idx].value;
            idx = (idx + 1) % (unsigned)_cap;
            if (idx == start) { if (wrapped) break; wrapped = true; }
        }
        return nullptr;
    }

    const V* find(const K& k) const {
        unsigned idx = slot0(k);
        unsigned start = idx;
        bool wrapped = false;
        while (_slots[idx].used) {
            if (!_slots[idx].deleted && _slots[idx].key == k)
                return &_slots[idx].value;
            idx = (idx + 1) % (unsigned)_cap;
            if (idx == start) { if (wrapped) break; wrapped = true; }
        }
        return nullptr;
    }

    bool contains(const K& k) const { return find(k) != nullptr; }

    // ── 删除 ─────────────────────────────────────────
    bool erase(const K& k) {
        unsigned idx = slot0(k);
        unsigned start = idx;
        bool wrapped = false;
        while (_slots[idx].used) {
            if (!_slots[idx].deleted && _slots[idx].key == k) {
                _slots[idx].deleted = true;
                --_size;
                return true;
            }
            idx = (idx + 1) % (unsigned)_cap;
            if (idx == start) { if (wrapped) break; wrapped = true; }
        }
        return false;
    }

    int  size()  const { return _size; }
    bool empty() const { return _size == 0; }

    void clear() {
        for (int i = 0; i < _cap; ++i) {
            _slots[i].used    = false;
            _slots[i].deleted = false;
        }
        _size = 0;
    }

    // ── 遍历：对每个有效条目调用 func(key, value) ────
    template<typename F>
    void forEach(F func) {
        for (int i = 0; i < _cap; ++i)
            if (_slots[i].used && !_slots[i].deleted)
                func(_slots[i].key, _slots[i].value);
    }

    template<typename F>
    void forEach(F func) const {
        for (int i = 0; i < _cap; ++i)
            if (_slots[i].used && !_slots[i].deleted)
                func(_slots[i].key, _slots[i].value);
    }
};
