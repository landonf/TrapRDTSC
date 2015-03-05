/*
 * Copyright (c) 2015 Landon Fuller <landon@landonf.org>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "ttl.hpp"

namespace ftk {

/* Forward declarations to support friend classes */
template <typename L, typename R> class either;

/**
 * Disjoint union type.
 */
template <typename L, typename R> class either {
public:
    typedef R Right;
    typedef L Left;

    /* Tags used to differentiate left and right values */
    struct left_tag {};
    struct right_tag {};

    /** Construct a left value */
    constexpr either (const right_tag, const R &right) : _is_right(true), _right(right) {}
    
    /** Construct a right value */
    constexpr either (const left_tag, const L &left) : _is_right(false), _left(left) {}

    /** Copy constructor */
    either (const either &other) : _is_right(other._is_right) {
        if (_is_right) {
            new (&_right) R(other._right);
        } else {
            new (&_left) L(other._left);
        }
    }
    
    ~either () {
        if (_is_right)
            _right.~R();
        else
            _left.~L();
    }
    
    /**
     * Perform a match on this instance, returning the result of the matching function.
     *
     * @param fnl Function to match on left value.
     * @param fnr Function to match on right value.
     */
    template<typename FnR, typename FnL> auto match (const FnL &fnl, const FnR &fnr) ->
    typename ttl::common_type<decltype(fnl(ttl::declval<L>())), decltype(fnr(ttl::declval<R>()))>::type
    {
        if (_is_right) {
            return fnr(_right);
        } else {
            return fnl(_left);
        }
    }

    /**
     * Perform an effectful match on this instance.
     *
     * @param fnl Function to match on left value.
     * @param fnr Function to match on right value.
     */
    template<typename FnR, typename FnL> void matchE (const FnL &fnl, const FnR &fnr) {
        if (_is_right) {
            fnr(_right);
        } else {
            fnl(_left);
        }
    }

//private:
    /** If true, constructed with a 'right' value. Otherwise 'left' */
    bool _is_right;
    union {
        R _right;
        L _left;
    };
};

/** Construct an either left value */
template <typename R, typename L> const either<typename ttl::decay<L>::type, typename ttl::decay<R>::type> make_left (L &&left) {
    using LT = typename ttl::decay<L>::type;
    using RT = typename ttl::decay<R>::type;
    using TagT = typename either<LT, RT>::left_tag;
    return either<LT, RT>(TagT(), ttl::forward<L>(left));
}

/** Construct an either right value */
template <typename L, typename R> const either<typename ttl::decay<L>::type, typename ttl::decay<R>::type> make_right (R &&right) {
    using LT = typename ttl::decay<L>::type;
    using RT = typename ttl::decay<R>::type;
    using TagT = typename either<LT, RT>::right_tag;
    return either<LT, RT>(TagT(), ttl::forward<R>(right));
}

/** Functor map over either */
template<class L, class R, class F> auto inline fmap(F f, const either<L, R> &either) -> decltype(make_right<L>(f(either._right))) {
    using U = typename ttl::decay<decltype(f(either._right))>::type;
    if (either._is_right) {
        return make_right<L>(f(either._right));
    } else {
        return make_left<U>(either._left);
    }
}

/** Monadic bind over either */
template<class L, class R, class F> auto inline bind(F f, const either<L, R> &either) -> decltype(make_right<L>(f(either._right)._right)) {
    using U = typename ttl::decay<decltype(f(either._right))>::type::Right;
    if (either._is_right) {
        return f(either._right);
    } else {
        return make_left<U>(either._left);
    }
}
}