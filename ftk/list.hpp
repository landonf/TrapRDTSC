/*
 * This file contains code originally written by Bartosz Milewski for his
 * Okasaki library (https://github.com/BartoszMilewski/Okasaki) -- the
 * code has been modified and further adapted for use in our
 * kernel environment.
 *
 * The original code -- and our modifications in this file -- is provided
 * under the following public domain grant:
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org>
 */

#pragma once

#include "shared_ptr.hpp"
#include "micro_stl.hpp"

namespace ftk {

template<class T> class fwd_list_iter;

template<class T>
class list
{
    struct Item {
        Item (T v, ttl::shared_ptr<const Item> tail) : _val(v), _next(ttl::move(tail)) {}
    
        // singleton
        explicit Item(T v) : _val(v) {}
        
        T _val;
        ttl::shared_ptr<const Item> _next;
    };

    friend Item;
    
    explicit list(ttl::shared_ptr<const Item> items) : _head(ttl::move(items)) {}
public:
    // Empty list
    list() : _size(0) {}

    // Cons
    list(T v, list const &tail) : _head(ttl::make_shared<const Item>(v, tail._head)), _size(tail._size + 1) {}

    // Singleton
    explicit list(T v) : _head(ttl::make_shared<const Item>(v)), _size(1) {}
    
    /** Copy and move support */
    list(const list &other) = default;
    list(list &&other) = default;

    list &operator= (const list &other) = default;
    list &operator= (list &&other) = default;

    // From initializer list
    list(ttl::initializer_list<T> init) : _size(init.size()) {
        // TODO - Verify that this actually works?
        for (auto it = init.end(); it != init.begin(); --it)
        {
            _head = ttl::make_shared<Item>(*it, _head);
        }
    }

    bool isEmpty() const { return !_head; } // conversion to bool

    T head () const {
        assert(!isEmpty());
        return _head->_val;
    }

    list tail () const {
        if (isEmpty())
            return *this;
        else
            return list(_head->_next);
    }

    list prepend (T v) const {
        return list(v, *this);
    }

    list take (int n) {
        if (n <= 0 || isEmpty()) return list();
        return tail().take(n - 1).prepend(head());
    }

    list insertedAt(int i, T v) const
    {
        if (i == 0)
            return pushed_front(v);
        else {
            assert(!isEmpty());
            return list<T>(head(), tail().insertedAt(i - 1, v));
        }
    }

    list removed(T v) const {
        if (isEmpty()) return list();
        if (v == head())
            return tail().removed(v);
        return list(head(), tail().removed(v));
    }

    list removed1(T v) const {
        if (isEmpty()) return list();
        if (v == head())
            return tail();
        return list(head(), tail().removed(v));
    }

    bool contains (T v) const {
        if (isEmpty()) return false;
        if (v == head()) return true;
        return tail().member(v);
    }

    template<class F> void foreach (F f) const {
        Item const * it = _head.get();
        while (it != nullptr)
        {
            f(it->_val);
            it = it->_next.get();
        }
    }
    
    size_t size () const { return _size; }
    
    friend class fwd_list_iter<T>;

    // For debugging
    size_t headCount() const { return _head.use_count(); }
private:
    ttl::shared_ptr<const Item> _head;
    size_t _size;
};

template<class T, class P>
bool all(list<T> const & lst, P & p)
{
    if (lst.isEmpty())
        return true;
    if (!p(lst.head()))
        return false;
    return all(lst.tail(), p);
}

template<class T>
class fwd_list_iter // TODO : public ttl::iterator<ttl::forward_iterator_tag, T>
{
public:
    fwd_list_iter() {} // end
    fwd_list_iter(list<T> const & lst) : _cur(lst._head)
    {}
    T operator*() const { return _cur->_val; }
    fwd_list_iter & operator++()
    {
        _cur = _cur->_next;
        return *this;
    }
    bool operator==(fwd_list_iter<T> const & other)
    {
        return _cur == other._cur;
    }
    bool operator!=(fwd_list_iter<T> const & other)
    {
        return !(*this == other);
    }
private:
    ttl::shared_ptr<const typename list<T>::Item> _cur;
};

template<class T>
class OutlistIter // TODO : public ttl::iterator<ttl::output_iterator_tag, T>
{
public:
    OutlistIter() {}
    T & operator*() { return _val; }
    OutlistIter & operator++()
    {
        _lst = list<T>(_val, _lst);
        return *this;
    }
    list<T> getlist() const { return _lst; }
private:
    T _val;
    list<T> _lst;
};


// TODO namespace ttl {
    template<class T> 
    fwd_list_iter<T> begin(list<T> const & lst)
    {
        return fwd_list_iter<T>(lst);
    }
    template<class T> 
    fwd_list_iter<T> end(list<T> const & lst)
    {
        return fwd_list_iter<T>();
    }
// TODO }

template<class T>
list<T> concat(list<T> const & a, list<T> const & b)
{
    if (a.isEmpty())
        return b;
    return list<T>(a.head(), concat(a.tail(), b));
}

template<class T, class F, class U = decltype(ttl::declval<F>()(ttl::declval<T>()))>
list<U> fmap(F f, list<T> lst)
{
#if 0
    static_assert(ttl::is_convertible<F, ttl::function<U(T)>>::value,
                 "fmap requires a function type U(T)");
#endif
    if (lst.isEmpty()) 
        return list<U>();
    else
        return list<U>(f(lst.head()), fmap(f, lst.tail()));
}

template<class T, class P>
list<T> filter(P p, list<T> lst)
{
#if 0
    static_assert(ttl::is_convertible<P, ttl::function<bool(T)>>::value,
                 "filter requires a function type bool(T)");
#endif
    if (lst.isEmpty())
        return list<T>();
    if (p(lst.head()))
        return list<T>(lst.head(), filter(p, lst.tail()));
    else
        return filter(p, lst.tail());
}

template<class T, class U, class F>
U foldr(F f, U acc, list<T> lst)
{
#if 0
    static_assert(ttl::is_convertible<F, ttl::function<U(T, U)>>::value,
                 "foldr requires a function type U(T, U)");
#endif
    if (lst.isEmpty())
        return acc;
    else
        return f(lst.head(), foldr(f, acc, lst.tail()));
}

template<class T, class U, class F>
U foldl(F f, U acc, list<T> lst)
{
#if 0
    static_assert(ttl::is_convertible<F, ttl::function<U(U, T)>>::value,
                 "foldl requires a function type U(U, T)");
 #endif
    if (lst.isEmpty())
        return acc;
    else
        return foldl(f, f(acc, lst.head()), lst.tail());
}

// Set difference a \ b
template<class T>
list<T> set_diff(list<T> const & as, list<T> const & bs)
{
    return foldl([](list<T> const & acc, T x) {
        return acc.removed(x);
    }, as, bs);
}

// Set union of two lists, xs u ys
// Assume no duplicates inside either list
template<class T>
list<T> set_union(list<T> const & xs, list<T> const & ys)
{
    // xs u ys = (ys \ xs) ++ xs
    // removed all xs from ys
    auto trimmed = foldl([](list<T> const & acc, T x) {
        return acc.removed(x);
    }, ys, xs);
    return concat(trimmed, xs);
}

template<class T>
list<T> concatAll(list<list<T>> const & xss)
{
    if (xss.isEmpty()) return list<T>();
    return concat(xss.head(), concatAll(xss.tail()));
}

// consumes the list when called: 
// forEach(ttl::move(lst), f);

template<class T, class F>
void forEach(list<T> lst, F f) 
{
#if 0
    static_assert(ttl::is_convertible<F, ttl::function<void(T)>>::value,
                 "forEach requires a function type void(T)");
#endif
    while (!lst.isEmpty()) {
        f(lst.head());
        lst = lst.tail();
    }
}

template<class Beg, class End>
auto fromIt(Beg it, End end) -> list<typename Beg::value_type>
{
    typedef typename Beg::value_type T;
    if (it == end)
        return list<T>();
    T item = *it;
    return list<T>(item, fromIt(++it, end));
}

template<class T, class F>
list<T> iterateN(F f, T init, int count)
{
    if (count <= 0) return list<T>();
    return iterateN(f, f(init), count - 1).pushed_front(init);
}

template<class T>
list<T> reversed(list<T> const & lst)
{
    return foldl([](list<T> const & acc, T v)
    {
        return list<T>(v, acc);
    }, list<T>(), lst);
}
    
} /* namespace ftk */
