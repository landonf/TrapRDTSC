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

#include <sys/types.h>

#include <ftk/ftk.hpp>
#include <ttl/ttl.hpp>
#include "IDT.hpp"

namespace traprdt {

/**
 * Represents a local system CPU.
 */
class CPU {
public:
    CPU (const uint32_t lapicID) : _lapicID(lapicID) {}
    
    /** Copy constructor */
    CPU (const CPU &other) = default;
    
    /** Move constructor */
    CPU (CPU &&other) = default;
    
    /** Copy assignment operator */
    CPU &operator= (const CPU &other) = default;
    
    /** Move assignment operator */
    CPU &operator= (CPU &&other) = default;
    
    /** Return the CPU lapic identifier returned by IOKit */
    uint64_t lapicID () const { return _lapicID; }
    
private:
    /**
     * CPUID return values.
     *
     * See also:
     * - Table 3-17. Information Returned by CPUID Instruction in Vol 2A of the Intel® 64 and IA-32 Architectures Software Developer’s Manual.
     */
    struct CPUID {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    } __attribute__((packed));
    static_assert(ttl::is_pod<CPUID>::value, "Must be a POD type.");
    
    /**
     * Issue a CPUID request
     *
     * @param request The initial EAX value.
     */
    static inline CPUID cpuid (uint32_t request) {
        CPUID cpuid;
        
        asm("cpuid"
            : "=a" (cpuid.eax), "=b" (cpuid.ebx), "=c" (cpuid.ecx), "=d" (cpuid.edx)
            : "a"(request), "b" (0), "c" (0), "d" (0));
        
        return cpuid;
    }
    
public:
    /**
     * Read the current CPU's local APIC/xAPIC identifier. Note that x2APIC uses 32-bit CPU identifiers,
     * but as per the Intel® 64 Architecture x2APIC Specification:
     *
     * "The legacy processor topology enumeration fields in CPUID.01H and CPUID.04H will continue to report
     * correct topology up to the maximum values supported by the fields and 8-bit initial APIC ID"
     *
     * I'm OK with this code failing on systems with > 256 processors.
     *
     * @warning The notion of "current CPU" is not particularly useful if you haven't also disabled interrupts.
     */
    static uint8_t current_lapicID () {
        const CPUID &result = cpuid(0x1);
        return ((result.ebx >> 24) & 0xFF);
    }

    /**
     * Read the current CPU's IDTR.
     *
     * @warning The notion of "current CPU" is not particularly useful if you haven't also disabled interrupts.
     */
    static IDTR sidt () {
        IDTR ret;
        asm (
             "sidt %0"
             :
             : "m" (ret)
         );
        
        return ret;
    }
    
    /**
     * Disable interrupts, and execute the provided function.
     */
    template <typename Fn> static inline auto disableInterrupts (const Fn &fn) -> decltype(fn()) {
        asm volatile ("cli");
        auto const ret = fn();
        asm volatile ("sti");

        return ret;
    }

    /**
     * Disable interrupts, clear the WP (write protect) bit in CR0, and execute the provided zero argument function.
     *
     * After execution, the previous value of CR0 will be restored and interrupts will be re-enabled and the
     * result of @a fn will be returned.
     *
     * @param fn Function to execute with interrupts and write protection disabled.
     */
    template <typename Fn> static inline auto disableWriteProtect (const Fn &fn) -> decltype(fn()) {
        uint64_t cr0;
        
        /* Disable interrupts, save cr0, clear the WP (write protect) bit in cr0. */
        asm volatile (
            "cli;"
            "mov    %%cr0, %%rax;"
            "mov    %%rax, %0;"
            "and    $0xfffffffffffeffff, %%rax;"
            "mov    %%rax, %%cr0"
            : "=r" (cr0) /* output  */
            :            /* input   */
            : "rax"      /* clobber */
        );

        /* Execute the provided function */
        const auto &ret = fn();
        
        /* Restore cr0, re-enable interrupts. */
        asm volatile (
            "mov    %0, %%cr0;"
            "sti;"
            :
            : "r" (cr0)
            : "rax"
        );
        
        return ret;
    }

private:
    /** IOKit-reported APIC ID of this CPU */
    uint32_t _lapicID;
};

}