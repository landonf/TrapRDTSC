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

#include <mach/mach_types.h>
#include "ftk.hpp"

using namespace ftk;

namespace traprdt {
    

/**
 * TrapRDTSC Error
 */
class Error {
public:
    /** Construct a new instance */
    Error (kern_return_t kt, const char *description) : _kt(kt), _description(description) {};

    /** Error description */
    const char *description () const { return _description; };
    
    /** Kernel error code. */
    kern_return_t kt () const { return _kt; }
        
private:
    kern_return_t _kt;
    const char *_description;
};

template <typename T> using Result = either<Error, T>;

/**
 * Construct a successful result.
 */
template <typename T> auto yield (T &&result) -> decltype(make_right<Error>(ttl::forward<T>(result))) {
    return make_right<Error>(ttl::forward<T>(result));
}

/**
 * Construct an error result.
 */
template <typename T> auto fail (const Error &error) -> decltype(make_left<T>(error)) {
    return make_left<T>(error);
}

/**
 * Construct a generic KERN_FAILURE error result.
 */
template <typename T> auto fail (const char *description) -> decltype(make_left<T>(Error(KERN_FAILURE, description))) {
    return make_left<T>(Error(KERN_FAILURE, description));
}

}