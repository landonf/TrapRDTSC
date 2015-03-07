/*
 * Author: Landon Fuller <landonf@plausiblelabs.com>
 *
 * Copyright (c) 2008-2013 Plausible Labs Cooperative, Inc.
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

#import <Foundation/Foundation.h>

/* Error Domain and Codes */
extern NSString *PLCrashReporterErrorDomain;

/**
 * NSError codes in the Plausible Crash Reporter error domain.
 * @ingroup enums
 */
typedef enum {
    /** An unknown error has occured. If this
     * code is received, it is a bug, and should be reported. */
    PLCrashReporterErrorUnknown = 0,
    
    /** An Mach or POSIX operating system error has occured. The underlying NSError cause may be fetched from the userInfo
     * dictionary using the NSUnderlyingErrorKey key. */
    PLCrashReporterErrorOperatingSystem = 1,
    
    /** The crash report log file is corrupt or invalid */
    PLCrashReporterErrorCrashReportInvalid = 2,
    
    /** An attempt to use a resource which was in use at the time in a manner which would have conflicted with the request. */
    PLCrashReporterErrorResourceBusy = 3,
    
    /** An unexpected internal error occured. */
    PLCrashReporterErrorInternal = 4
} PLCrashReporterError;

void plcrash_populate_error (NSError **error, PLCrashReporterError code, NSString *description, NSError *cause);
void plcrash_populate_mach_error (NSError **error, kern_return_t kr, NSString *description);
void plcrash_populate_posix_error (NSError **error, int errnoVal, NSString *description);
