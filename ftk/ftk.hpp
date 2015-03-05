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

#include "either.hpp"
#include "list.hpp"
#include "unit.hpp"

/**
 * Operator syntax for bind(monad, fn).
 *
 * @a monad The monad over which @a fn will be bound.
 * @a fn A function to bind (flat map) over @a monad.
 */
template<typename F, typename R> auto operator>>= (const R &monad, const F &fn) -> decltype(bind(fn, monad)) {
    return bind(fn, monad);
};

/**
 * Operator syntax for fmap(functor, fn).
 *
 * @a monad The monad over which @a fn will be bound.
 * @a fn A function to map (map) over @a functor.
 */
template<typename F, typename R> auto operator>> (const R &functor, const F &fn) -> decltype(fmap(fn, functor)) {
    return fmap(fn, functor);
};