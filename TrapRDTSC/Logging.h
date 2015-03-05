/*
 * Copyright (c) 2012-2015 Landon Fuller <landon@landonf.org>
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

/* Define to 0 to disable debug logging */
#define TRAP_RDTSC_DEBUG_LOG 1

/** Log a message. It will be automatically prefixed with the driver name. */
#define TRLog(fmt,...) do { \
    IOLog("TrapRDTSC: " fmt "\n", ## __VA_ARGS__); \
} while (0)

/** Log an error message. */
#define TRLogError(fmt,...) TRLog("[ERROR] " fmt, ## __VA_ARGS__)

/** Log an informative message. */
#define TRLogInfo(fmt,...) TRLog(fmt, ## __VA_ARGS__)

#if TRAP_RDTSC_DEBUG_LOG
/** Log a debugging message. */
#define TRLogDebug(fmt, ...) TRLog("[DEBUG] " fmt, ## __VA_ARGS__)
#else /* CP210x_DEBUG */
#define TRLogDebug(...) do { } while (0)
#endif /* !CP210x_DEBUG */