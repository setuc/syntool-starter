#ifndef TILECACHE_H
#define TILECACHE_H

#include <QtCore/qhash.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


template <class Key, class T>
class TileCache
{
    struct Node {
        inline Node() : keyPtr(0) {}
        inline Node(T *data, int cost)
            : keyPtr(0), t(data), c(cost), p(0), n(0) {}
        const Key *keyPtr; T *t; int c; Node *p,*n;
    };
    Node *f, *l;
    QHash<Key, Node> hash;
    int mx, total;

    inline void unlink(Node &n) {
        if (n.p) n.p->n = n.n;
        if (n.n) n.n->p = n.p;
        if (l == &n) l = n.p;
        if (f == &n) f = n.n;
        total -= n.c;
        T *obj = n.t;
        hash.remove(*n.keyPtr);
        delete obj;
    }
    inline T *relink(const Key &key) {
        typename QHash<Key, Node>::iterator i = hash.find(key);
        if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd())
            return 0;

        Node &n = *i;
        if (f != &n) {
            if (n.p) n.p->n = n.n;
            if (n.n) n.n->p = n.p;
            if (l == &n) l = n.p;
            n.p = 0;
            n.n = f;
            f->p = &n;
            f = &n;
        }
        return n.t;
    }

    Q_DISABLE_COPY(TileCache)

public:
    inline explicit TileCache(int maxCost = 100);
    inline ~TileCache() { clear(); }

    inline int maxCost() const { return mx; }
    void setMaxCost(int m);
    inline int totalCost() const { return total; }

    inline int size() const { return hash.size(); }
    inline int count() const { return hash.size(); }
    inline bool isEmpty() const { return hash.isEmpty(); }
    inline QList<Key> keys() const { return hash.keys(); }

    void clear();

    bool insert(const Key &key, T *object, int cost = 1);
    T *object(const Key &key) const;
    inline bool contains(const Key &key) const { return hash.contains(key); }
    T *operator[](const Key &key) const;

    bool remove(const Key &key);
    T *take(const Key &key);

private:
    void trim(int m);
};

template <class Key, class T>
inline TileCache<Key, T>::TileCache(int amaxCost)
    : f(0), l(0), mx(amaxCost), total(0) {}

template <class Key, class T>
inline void TileCache<Key,T>::clear()
{ while (f) { delete f->t; f = f->n; }
 hash.clear(); l = 0; total = 0; }

template <class Key, class T>
inline void TileCache<Key,T>::setMaxCost(int m)
{ mx = m; trim(mx); }

template <class Key, class T>
inline T *TileCache<Key,T>::object(const Key &key) const
{ return const_cast<TileCache<Key,T>*>(this)->relink(key); }

template <class Key, class T>
inline T *TileCache<Key,T>::operator[](const Key &key) const
{ return object(key); }

template <class Key, class T>
inline bool TileCache<Key,T>::remove(const Key &key)
{
    typename QHash<Key, Node>::iterator i = hash.find(key);
    if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd()) {
        return false;
    } else {
        unlink(*i);
        return true;
    }
}

template <class Key, class T>
inline T *TileCache<Key,T>::take(const Key &key)
{
    typename QHash<Key, Node>::iterator i = hash.find(key);
    if (i == hash.end())
        return 0;

    Node &n = *i;
    T *t = n.t;
    n.t = 0;
    unlink(n);
    return t;
}

template <class Key, class T>
bool TileCache<Key,T>::insert(const Key &akey, T *aobject, int acost)
{
    remove(akey);
    if (acost > mx) {
        delete aobject;
        return false;
    }
    trim(mx - acost);
    Node sn(aobject, acost);
    typename QHash<Key, Node>::iterator i = hash.insert(akey, sn);
    total += acost;
    Node *n = &i.value();
    n->keyPtr = &i.key();
    if (f) f->p = n;
    n->n = f;
    f = n;
    if (!l) l = f;
    return true;
}

template <class Key, class T>
void TileCache<Key,T>::trim(int m)
{
    Node *n = l;
    while (n && total > m) {
        Node *u = n;
        n = n->p;
        unlink(*u);
    }
}

QT_END_NAMESPACE

QT_END_HEADER

//#include <QCache>

//template <class Key, class T>
//class TileCache : public QCache<Key, T>
//{
//    struct Node {
//        inline Node() : keyPtr(0) {}
//        inline Node(T *data, int cost)
//            : keyPtr(0), t(data), c(cost), p(0), n(0) {}
//        const Key *keyPtr; T *t; int c; Node *p,*n;
//    };
//    Node *f, *l;
//    QHash<Key, Node> hash;
//    int mx, total;

//    inline void unlink(Node &n) {
//        if (n.p) n.p->n = n.n;
//        if (n.n) n.n->p = n.p;
//        if (l == &n) l = n.p;
//        if (f == &n) f = n.n;
//        total -= n.c;
//        T *obj = n.t;
//        hash.remove(*n.keyPtr);
//        delete obj;
//    }

//public:
//    bool insert(const Key &key, T *object, int cost = 1);
//    inline explicit TileCache(int maxCost = 100);
//    bool remove(const Key &key);
//    inline bool contains(const Key &key) const { return hash.contains(key); }

//private:
//    void trim(int m);
//};

//template <class Key, class T>
//inline TileCache<Key, T>::TileCache(int amaxCost)
//    : f(0), l(0), mx(amaxCost), total(0)
//{
//}

//template <class Key, class T>
//inline bool TileCache<Key,T>::insert(const Key &akey, T *aobject, int acost)
//{
//    remove(akey);
//    if (acost > mx) {
//        delete aobject;
//        return false;
//    }
//    trim(mx - acost);
//    Node sn(aobject, acost);
//    typename QHash<Key, Node>::iterator i = hash.insert(akey, sn);
//    total += acost;
//    Node *n = &i.value();
//    n->keyPtr = &i.key();
//    if (f) f->p = n;
//    n->n = f;
//    f = n;
//    if (!l) l = f;
//    return true;
//}

//template <class Key, class T>
//inline bool TileCache<Key,T>::remove(const Key &key)
//{
//    typename QHash<Key, Node>::iterator i = hash.find(key);
//    if (typename QHash<Key, Node>::const_iterator(i) == hash.constEnd()) {
//        return false;
//    } else {
//        unlink(*i);
//        return true;
//    }
//}

//template <class Key, class T>
//inline void TileCache<Key,T>::trim(int m)
//{
//    Node *n = l;
//    while (n && total > m) {
//        Node *u = n;
//        n = n->p;
//        unlink(*u);
//    }
//}

#endif // TILECACHE_H
