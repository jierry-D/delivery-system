#pragma once
// 自实现容器库（替代 STL vector / map / priority_queue / queue）
#include <utility>
#include <string>

// ════════════════════════════════════════════════════
//  DynArray<T>  —  动态数组（替代 std::vector）
// ════════════════════════════════════════════════════
template<typename T>
class DynArray {
    T* _d = nullptr;
    int _sz = 0, _cap = 0;

    void grow() {
        int nc = _cap ? _cap * 2 : 4;
        T* nd = new T[nc];
        for (int i = 0; i < _sz; ++i) nd[i] = std::move(_d[i]);
        delete[] _d; _d = nd; _cap = nc;
    }
public:
    DynArray() = default;
    DynArray(const DynArray& o) { for (int i=0;i<o._sz;++i) push_back(o._d[i]); }
    DynArray(DynArray&& o) noexcept : _d(o._d),_sz(o._sz),_cap(o._cap)
        { o._d=nullptr; o._sz=o._cap=0; }
    ~DynArray() { delete[] _d; }

    DynArray& operator=(DynArray o) noexcept {    // copy-and-swap
        std::swap(_d,o._d); std::swap(_sz,o._sz); std::swap(_cap,o._cap);
        return *this;
    }

    void push_back(const T& v) { if (_sz==_cap) grow(); _d[_sz++]=v; }
    void push_back(T&& v)      { if (_sz==_cap) grow(); _d[_sz++]=std::move(v); }
    void pop_back()            { if (_sz>0) --_sz; }

    T& operator[](int i)       { return _d[i]; }
    const T& operator[](int i) const { return _d[i]; }
    T& back()             { return _d[_sz-1]; }
    int  size()  const    { return _sz; }
    bool empty() const    { return _sz==0; }
    void clear()          { _sz=0; }

    void erase(int i) {
        for (; i<_sz-1; ++i) _d[i]=std::move(_d[i+1]); --_sz;
    }
    template<typename P> bool remove_first(P pred) {
        for (int i=0;i<_sz;++i) if (pred(_d[i])) { erase(i); return true; }
        return false;
    }
    template<typename P> void remove_all(P pred) {
        for (int i=0;i<_sz;) { if(pred(_d[i])) erase(i); else ++i; }
    }

    T* begin() { return _d; }
    T* end()   { return _d+_sz; }
    const T* begin() const { return _d; }
    const T* end()   const { return _d+_sz; }
};

// ════════════════════════════════════════════════════
//  HashMap<K,V>  —  哈希表（替代 std::map / unordered_map）
//  开放地址 + 线性探测
// ════════════════════════════════════════════════════
template<typename K, typename V>
class HashMap {
    struct Slot { K key; V val; bool used=false, del=false; };
    Slot* _s; int _cap, _sz;

    unsigned h(int k)               const { return (unsigned)k*2654435761u % (unsigned)_cap; }
    unsigned h(const std::string& k) const {
        unsigned r=5381; for(char c:k) r=r*33+(unsigned char)c; return r%(unsigned)_cap;
    }
    unsigned slot0(const K& k) const {
        if constexpr(std::is_same_v<K,int>) return h((int)k);
        else return h(k);
    }
    void rehash(int nc) {
        Slot* old=_s; int oc=_cap;
        _s=new Slot[nc](); _cap=nc; _sz=0;
        for (int i=0;i<oc;++i) if(old[i].used&&!old[i].del) (*this)[old[i].key]=std::move(old[i].val);
        delete[] old;
    }
public:
    explicit HashMap(int c=16): _s(new Slot[c]()), _cap(c), _sz(0) {}
    HashMap(const HashMap& o): _s(new Slot[o._cap]()), _cap(o._cap), _sz(o._sz)
        { for(int i=0;i<_cap;++i) _s[i]=o._s[i]; }
    HashMap(HashMap&& o) noexcept: _s(o._s),_cap(o._cap),_sz(o._sz)
        { o._s=nullptr; o._cap=o._sz=0; }
    ~HashMap() { delete[] _s; }
    HashMap& operator=(HashMap o) noexcept {
        std::swap(_s,o._s); std::swap(_cap,o._cap); std::swap(_sz,o._sz); return *this;
    }

    V& operator[](const K& k) {
        if (_sz*2>=_cap) rehash(_cap*2);
        unsigned i=slot0(k); int fd=-1;
        while (_s[i].used) {
            if (_s[i].del) { if(fd<0) fd=(int)i; }
            else if (_s[i].key==k) return _s[i].val;
            i=(i+1)%(unsigned)_cap;
        }
        int ins=(fd>=0)?fd:(int)i;
        _s[ins]={k,V{},true,false}; ++_sz;
        return _s[ins].val;
    }
    void set(const K& k, const V& v) { (*this)[k]=v; }
    void set(const K& k, V&& v)      { (*this)[k]=std::move(v); }

    V* find(const K& k) {
        unsigned i=slot0(k), s=i;
        while (_s[i].used) {
            if (!_s[i].del&&_s[i].key==k) return &_s[i].val;
            if ((i=(i+1)%(unsigned)_cap)==s) break;
        }
        return nullptr;
    }
    const V* find(const K& k) const {
        unsigned i=slot0(k), s=i;
        while (_s[i].used) {
            if (!_s[i].del&&_s[i].key==k) return &_s[i].val;
            if ((i=(i+1)%(unsigned)_cap)==s) break;
        }
        return nullptr;
    }
    bool contains(const K& k) const { return find(k)!=nullptr; }
    bool erase(const K& k) {
        unsigned i=slot0(k), s=i;
        while (_s[i].used) {
            if (!_s[i].del&&_s[i].key==k) { _s[i].del=true; --_sz; return true; }
            if ((i=(i+1)%(unsigned)_cap)==s) break;
        }
        return false;
    }
    int  size()  const { return _sz; }
    bool empty() const { return _sz==0; }
    void clear() { for(int i=0;i<_cap;++i){_s[i].used=_s[i].del=false;} _sz=0; }

    template<typename F> void forEach(F f) {
        for(int i=0;i<_cap;++i) if(_s[i].used&&!_s[i].del) f(_s[i].key,_s[i].val);
    }
    template<typename F> void forEach(F f) const {
        for(int i=0;i<_cap;++i) if(_s[i].used&&!_s[i].del) f(_s[i].key,_s[i].val);
    }
};

// ════════════════════════════════════════════════════
//  MinHeap<T,Cmp>  —  最小堆（替代 std::priority_queue）
// ════════════════════════════════════════════════════
template<typename T>
struct HeapLess { bool operator()(const T& a,const T& b) const { return a<b; } };

template<typename T, typename Cmp=HeapLess<T>>
class MinHeap {
    DynArray<T> _d; Cmp _c;
    void up(int i) {
        while (i>0) { int p=(i-1)/2; if(_c(_d[i],_d[p])){std::swap(_d[i],_d[p]);i=p;}else break; }
    }
    void down(int i) {
        int n=_d.size();
        while (true) {
            int b=i,l=2*i+1,r=2*i+2;
            if(l<n&&_c(_d[l],_d[b])) b=l;
            if(r<n&&_c(_d[r],_d[b])) b=r;
            if(b==i) break; std::swap(_d[i],_d[b]); i=b;
        }
    }
public:
    bool     empty() const { return _d.empty(); }
    const T& top()   const { return _d[0]; }
    void push(const T& v)  { _d.push_back(v); up(_d.size()-1); }
    void pop() { if(_d.empty()) return; _d[0]=std::move(_d.back()); _d.pop_back(); if(!_d.empty()) down(0); }
};

// ════════════════════════════════════════════════════
//  Queue<T>  —  先进先出队列（替代 std::queue）
// ════════════════════════════════════════════════════
template<typename T>
class Queue {
    DynArray<T> _d; int _h=0;
public:
    bool empty() const    { return _h>=_d.size(); }
    T&   front()          { return _d[_h]; }
    void push(const T& v) { _d.push_back(v); }
    void pop()            { if(!empty()) ++_h; }
    void clear()          { _d.clear(); _h=0; }
};
