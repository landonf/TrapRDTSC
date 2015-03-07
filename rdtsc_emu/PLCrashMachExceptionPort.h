/*
 * Author: Landon Fuller <landon@landonf.org>
 *
 * Derived from PLCrashReporter's mach exception handler.
 *
 * Copyright (c) 2015 Landon Fuller <landon@landonf.org>
 * Copyright (c) 2012-2015 Plausible Labs Cooperative, Inc.
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
#import <mach/mach.h>

#import "PLCrashMachExceptionPortSet.h"

@interface PLCrashMachExceptionPort : NSObject {
@private
    /** Exception mask. */
    exception_mask_t _mask;
    
    /** Exception server port. */
    mach_port_t _port;
    
    /** Exception behavior. */
    exception_behavior_t _behavior;
    
    /** Exception thread flavor. */
    thread_state_flavor_t _flavor;
}

- (instancetype) initWithServerPort: (mach_port_t) port
                               mask: (exception_mask_t) mask
                           behavior: (exception_behavior_t) behavior
                             flavor: (thread_state_flavor_t) flavor;

+ (PLCrashMachExceptionPortSet *) exceptionPortsForTask: (task_t) task mask: (exception_mask_t) mask error: (NSError **) outError;
+ (PLCrashMachExceptionPortSet *) exceptionPortsForThread: (thread_t) thread mask: (exception_mask_t) mask error: (NSError **) outError;

- (BOOL) registerForTask: (task_t) task previousPortSet: (PLCrashMachExceptionPortSet **) ports error: (NSError **) outError;
- (BOOL) registerForThread: (thread_t) thread previousPortSet: (PLCrashMachExceptionPortSet **) ports error: (NSError **) outError;

/** Exception server port. */
@property(nonatomic, readonly) mach_port_t server_port;

/** Exception mask. */
@property(nonatomic, readonly) exception_mask_t mask;

/** Exception behavior. */
@property(nonatomic, readonly) exception_behavior_t behavior;

/** Exception thread flavor. */
@property(nonatomic, readonly) thread_state_flavor_t flavor;


@end
