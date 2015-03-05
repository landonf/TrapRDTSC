/*
 * Copyright (c) 2014 Plausible Labs Cooperative, Inc.
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

#include "Reference.hpp"

namespace ttl {
    
/**
 * The unique_ptr class maintains sole ownership of an object, and destroys that object when
 * the unique_ptr instance goes out of scope.
 *
 * A unique_ptr may be empty, in which case it manages no object.
 *
 * A unique_ptr's ownership may be transfered to another unique_ptr, or a strong shared_ptr, leaving the original
 * unique_ptr empty.
 *
 * @tparam T The referenced object type.
 *
 * @par Thread Safety
 *
 * A single unique_ptr instance must not be concurrently mutated -- or accessed during mutation -- without
 * external synchronization.
 */
template <typename T> class unique_ptr {
public:
    /**
     * Construct an empty unique reference.
     */
    constexpr unique_ptr() noexcept { }
    
    /** Release ownership of the managed object, if any, and all associated resources. Upon return, the unique
     * reference will be empty. */
    inline void clear () {
        if (_ptr != nullptr) {
            delete _ptr;
            _ptr = nullptr;
        }
    }
    
private:
    /** Backing reference. */
    T *_ptr;
};
    
} /* namespace ttl */