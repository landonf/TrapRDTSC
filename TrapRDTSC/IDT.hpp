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

#include <ttl/ttl.hpp>
#include <libkern/libkern.h>
#include <libkern/OSAtomic.h>

static_assert(__LP64__ && __x86_64__, "This code assumes x86-64");

namespace traprdt {
    
/** Interrupt vector type */
typedef uint8_t ivec_t;

/**
 * 64-bit IDT gate descriptor.
 *
 * See also:
 *  - 64-bit Mode IDT -- Section 6.14.1 in Vol 3A of the Intel® 64 and IA-32 Architectures Software Developer’s Manual
 *  - http://wiki.osdev.org/Interrupt_Descriptor_Table
 */
struct IDT {
    /** NMI */
    static constexpr ivec_t INT_NMI = 2;
    
    /** INT 3 breakpoint */
    static constexpr ivec_t INT_BREAKPOINT = 3;
    
    /** General protection fault */
    static constexpr ivec_t INT_GP = 13;

    /**
     * Return the address of the handler for this IDT.
     */
    uintptr_t handler () const {
        return ((uintptr_t) offset_low          )  |
               ((uintptr_t) offset_mid  << 16   )  |
               ((uintptr_t) offset_high << 32ULL);
    };

    /**
     * Insert a new handler, placing the previous handler's address in @a previous.
     *
     * Note that this behavior is not and cannot be atomic.
     *
     * @warning This code assumes that the current IDT is valid and correctly configured
     * as to allow directly swapping in the replacement implementation address.
     */
    void patch (uintptr_t replacement, uintptr_t *previous) {
        if (previous != NULL)
            *previous = handler();
        offset_low  = (replacement & 0xFFFF);
        offset_mid  = (replacement >> 16) & 0xFFFF;
        offset_high = (replacement >> 32);
        
        __sync_synchronize();
    };
    
    /** Bits 0..15 of the handler entry point address */
    uint16_t    offset_low;
    
    /** Segment selector for destination code segment */
    uint16_t    segement_selector;
    
    /** Interrupt stack table. */
    uint8_t     ist:2;
    
    /** Zero padding */
    uint8_t     zero1:6;
    
    /** Type */
    uint8_t     type:4;
    
    /** Zero padding */
    uint8_t     zero2:1;
    
    /** Descriptor privilege level */
    uint8_t     dpl:2;
    
    /** Segment present flag */
    uint8_t     segment_present:1;

    /** Bits 16..31 of the handler entry point address */
    uint16_t    offset_mid;
    
    /** Bits 32..63 of the handler entry point address */
    uint32_t    offset_high;

    /** Reserved. */
    uint32_t    reserved;
} __attribute__((packed));
static_assert(ttl::is_pod<IDT>::value, "A must be a POD type.");

/**
 * 64-bit IDT reference, as used by LIDT/SIDT.
 *
 * See also:
 *  - 64-bit Mode IDT -- Section 6.14.1 in Vol 3A of the Intel® 64 and IA-32 Architectures Software Developer’s Manual
 *  - http://wiki.osdev.org/Interrupt_Descriptor_Table
 */
struct IDTR {
    /**
     * Insert a new handler, placing the previous handler's address in @a previous.
     *
     * Note that this behavior is not and cannot be atomic.
     *
     * @warning This code assumes that the current IDT exists, is valid, and is correctly
     * configured as to allow directly swapping in the replacement implementation address.
     */
    void patch (ivec_t vector, uintptr_t replacement, uintptr_t *previous) {
        assert(size() > vector);
        (&descriptors()[vector])->patch(replacement, previous);
    }

    /**
     * Return the size of the IDT table (the number of IDT descriptors);
     */
    size_t size () { return limit / sizeof(IDT); }
    
    /**
     * Return a direct reference to the IDT table.
     */
    IDT *descriptors () { return (IDT *)(address); }

    /* Maximum addressable byte in the interrupt descriptor table */
    uint16_t limit;
    
    /** Linear address of IDT */
    uint64_t address;
} __attribute__((packed));
static_assert(ttl::is_pod<IDTR>::value, "A must be a POD type.");

} /* namespace traprdt */