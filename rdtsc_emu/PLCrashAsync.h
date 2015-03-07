/*
 * Author: Landon Fuller <landonf@plausible.coop>
 *
 * Copyright (c) 2015 Landon Fuller <landon@landonf.org>
 * Copyright (c) 2008-2015 Plausible Labs Cooperative, Inc.
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

/* PLCrashReporter compatibility types/defines */
#include <assert.h>

#include "Logging.h"
#define PLCF_DEBUG(...) TRLogDebug(__VA_ARGS__)

#define PLCF_ASSERT(expr) assert(expr)

/**
 * @ingroup plcrash_async
 * Error return codes.
 */
typedef enum  {
    /** Success */
    PLCRASH_ESUCCESS = 0,
    
    /** Unknown error (if found, is a bug) */
    PLCRASH_EUNKNOWN,
    
    /** The output file can not be opened or written to */
    PLCRASH_OUTPUT_ERR,
    
    /** No memory available (allocation failed) */
    PLCRASH_ENOMEM,
    
    /** Unsupported operation */
    PLCRASH_ENOTSUP,
    
    /** Invalid argument */
    PLCRASH_EINVAL,
    
    /** Internal error */
    PLCRASH_EINTERNAL,
    
    /** Access to the specified resource is denied. */
    PLCRASH_EACCESS,
    
    /** The requested resource could not be found. */
    PLCRASH_ENOTFOUND,
    
    /** The input data is in an unknown or invalid format. */
    PLCRASH_EINVALID_DATA,
} plcrash_error_t;

const char *plcrash_async_strerror (plcrash_error_t error);