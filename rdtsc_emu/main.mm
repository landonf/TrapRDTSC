/*
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

#include <dispatch/dispatch.h>

#include <mach/mach_vm.h>

#include <sys/types.h>
#include <sys/sysctl.h>

#include "PLCrashMachExceptionServer.h"
#include "PLCrashAsyncThread.h"
#include "Logging.h"

static plcrash_mach_exception_port_set_t orig_port_set;

/* MIB used to fetch the current TSC value from the kernel */
static int rdtsc_mib[3];

/**
 * Safely add @a offset to @a base_address, returning the result in @a result. If an overflow would occur, false is returned.
 *
 * @param base_address The base address from which @a result will be computed.
 * @param offset The offset to apply to @a base_address.
 * @param result The location in which to store the result.
 */
bool plcrash_async_address_apply_offset (mach_vm_address_t base_address, mach_vm_offset_t offset, mach_vm_address_t *result) {
    /* Check for overflow */
    if (offset > 0 && UINT64_MAX - offset < base_address) {
        return false;
    }
    
    if (result != NULL)
        *result = base_address + offset;
    
    return true;
}

/**
 * Copy @a len bytes from @a task, at @a address + @a offset, storing in @a dest. If the page(s) at the
 * given @a address + @a offset are unmapped or unreadable, no copy will be performed and an error will
 * be returned.
 *
 * @param task The task from which data from address @a source will be read.
 * @param address The base address within @a task from which the data will be read.
 * @param offset The offset from @a address at which data will be read.
 * @param dest The destination address to which copied data will be written.
 * @param len The number of bytes to be read.
 *
 * @return Returns true on success, false on error.
 */
bool plcrash_async_task_memcpy (mach_port_t task, mach_vm_address_t address, mach_vm_offset_t offset, void *dest, mach_vm_size_t len) {
    mach_vm_address_t target;
    kern_return_t kt;
    
    /* Compute the target address and check for address */
    if (offset > 0 && UINT64_MAX - offset < address)
        return false;
    
    target = address + offset;
    
    mach_vm_size_t read_size = len;
    kt = mach_vm_read_overwrite(task, target, len, (pointer_t) dest, &read_size);
    if (kt != KERN_SUCCESS)
        return false;

    return true;
}

static kern_return_t rdtsc_exception_handler (task_t task, thread_t thread, exception_type_t exception_type, mach_exception_data_t code, mach_msg_type_number_t code_count, void *context) {
    TRLogDebug("handler hit");
    
    plcrash_gen_regnum_t edx, eax, ecx;
    plcrash_error_t err;
    plcrash_greg_t ip;
    plcrash_greg_t instr_size;
    size_t tsc_size;
    uint64_t tsc;
    
    kern_return_t kt;

    bool rdtscp = false;
    uint8_t instr[3] = { 0 };

#if 0
    /* GP should map to one of these*/
    if (exception_type != EXC_BAD_ACCESS && exception_type != EXC_BAD_INSTRUCTION)
        goto forwarding;
    
    /* rdtsc will trigger a GPFLT with an error code of 0 (as per the Instruction Set Reference in Vol 2B of the Intel® 64 and
     * IA-32 Architectures Software Developer’s Manual) */
    if (code_count != 2 || code[0] != EXC_I386_GPFLT || code[1] != 0)
        goto forwarding;
#endif
    
    /* We have to fetch the thread's full state before we can examine RIP */
    plcrash_async_thread_state thr_state;
    err = plcrash_async_thread_state_mach_thread_init(&thr_state, thread);
    if (err != PLCRASH_ESUCCESS) {
        TRLogError("Fetch of x86 thread state failed: %s", plcrash_async_strerror(err));
        goto forwarding;
    }

    /* Fetch the current IP */
    ip = plcrash_async_thread_state_get_reg(&thr_state, PLCRASH_REG_IP);

    /* Fetch 2-3 instructions at IP */
    if (plcrash_async_task_memcpy(task, ip, 0, &instr, sizeof(instr)) == false) {
        if (plcrash_async_task_memcpy(task, ip, 0, &instr, sizeof(instr) - 1) == false) {
            TRLogError("Fetch of instructions at faulting IP failed");
            goto forwarding;
        }
    }

    /* Check for either rdtsc or rdtscp */
    if (instr[0] == 0x0F && instr[1] == 0x31) {
        /* rdtsc */
        instr_size = 2;
    } else if (instr[0] == 0x0F && instr[1] == 0x01 && instr[2] == 0xf9) {
        /* rdtscp */
        instr_size = 3;
        rdtscp = true;
    } else {
        goto forwarding;
    }
    
    /* Determine the appropriate target registers for 32-bit or 64-bit thread state */
    if (thr_state.x86_state.thread.tsh.flavor == x86_THREAD_STATE32) {
        edx = (plcrash_gen_regnum_t) PLCRASH_X86_EDX;
        eax = (plcrash_gen_regnum_t) PLCRASH_X86_EAX;
        ecx = (plcrash_gen_regnum_t) PLCRASH_X86_ECX;
    } else {
        edx = (plcrash_gen_regnum_t) PLCRASH_X86_64_RDX;
        eax = (plcrash_gen_regnum_t) PLCRASH_X86_64_RAX;
        ecx = (plcrash_gen_regnum_t) PLCRASH_X86_64_RCX;
    }
    
    /* Insert the real rdtsc(p) value */
    tsc_size = sizeof(tsc);
    if (sysctl(rdtsc_mib, 3, &tsc, &tsc_size, nullptr, 0) != 0) {
        TRLogError("sysctl() failed: %s", strerror(errno));
        goto forwarding;
    }
    
    /* Populate the results */
    plcrash_async_thread_state_set_reg(&thr_state, edx, (tsc >> 32)); // high 32 bits
    plcrash_async_thread_state_set_reg(&thr_state, eax, (tsc & 0xFFFFFFFF)); // low 32 bits

    // TODO: IA32_TSC_AUX MSR should be placed in ecx
    if (rdtscp)
        plcrash_async_thread_state_set_reg(&thr_state, ecx, 7331);

    /* Advance the thread past the faulting instruction. */
    plcrash_async_thread_state_set_reg(&thr_state, PLCRASH_REG_IP, ip + instr_size);
    
    /* Apply the new thread state */
    kt = thread_set_state(thread, x86_THREAD_STATE, (thread_state_t) &thr_state.x86_state, x86_THREAD_STATE_COUNT);
    if (kt != KERN_SUCCESS) {
        TRLogError("thread_set_state() failed: %d", kt);
        goto forwarding;
    }
    
    /* Inform the kernel that the fault was handled */
    return KERN_SUCCESS;

    
forwarding:
    /* Forward to any previously registered handlers */
    return PLCrashMachExceptionForward(task, thread, exception_type, code, code_count, &orig_port_set);
}

int main(int argc, const char * argv[]) {
    NSError *error;
    PLCrashMachExceptionPortSet *previousPortSet;
    
    size_t mib_size = 3;
    if (sysctlnametomib("debug.trap_rdtsc.tsc", rdtsc_mib, &mib_size) != 0) {
        TRLogError("Could not find TrapRDTSC sysctl MIB; is the kext loaded? %s", strerror(errno));
        return 1;
    }

    /* Create the server */
    const exception_mask_t exc_mask = EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION;
    PLCrashMachExceptionServer *server = [[PLCrashMachExceptionServer alloc] initWithCallBack:rdtsc_exception_handler
                                                                                      context:nullptr
                                                                                        error:&error];
    if (server == nil) {
        TRLogError("Failed to instantiate the Mach exception server: %s", error.description.UTF8String);
        return 1;
    }
    
    /* Allocate the port */
    PLCrashMachExceptionPort *port = [server exceptionPortWithMask: exc_mask error: &error];
    if (port == nil) {
        TRLogError("Failed to instantiate the Mach exception port: %s", error.description.UTF8String);
        return 1;
    }
    
    /* Register for the host */
    host_t host_port = mach_host_self();
    if (![port registerForHost: host_port previousPortSet: &previousPortSet error: &error]) {
        TRLogError("Failed to set the host's mach exception ports: %s", error.description.UTF8String);
        return 1;
    }
    mach_port_deallocate(mach_task_self(), host_port);
    
    /*
     * Save the previous ports.
     *
     * XXXTODO: There's a race condition here, in that an exception that is delivered before (or during)
     * setting the previous port values will see a fully and/or partially configured port set.
     */
    orig_port_set = previousPortSet.asyncSafeRepresentation;

    TRLogInfo("rdtsc emulation server is running");
    
    int flag = 1;
    sysctlbyname("debug.trap_rdtsc.time_stamp_disable", nullptr, nullptr, &flag, sizeof(flag));

    while (1) sleep(10);
    return 0;
}
