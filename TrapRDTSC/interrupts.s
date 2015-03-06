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

 /* 
  * Original ISR lookup macro.
  *
  * Fetches the address of the original ISR and save it in the designated destination.
  *
  * Arguments:
  *     $0 - Interrupt vector.
  *     $1 - Destination register.
  */
 .macro lookup_orig_isr
    movq ($0 * 8) + _traprdt_orig_isr(%rip), $1 // vector * sizeof(uintptr_t)
 .endmacro

 /*
  * Switch to the section containing our unwind table.
  *
  * We implement internal page fault recovery by managing an "unwind" table
  * of (rip, handler) pairs, jumping to the registered handler if the rip is
  * found.
  *
  * The table and entries are defined using the unwind_* macros below.
  *
  * Imagine if eh_frame unwinding was this simple ...
  */
.macro UNWIND_SECT
    .section __TEXT, __trdtsc_unwind
.endmacro

 /*
  * Define a new unwind entry.
  *
  * Arguments:
  *     $0 - Unwind handler
  *
  * Labels:
  *     10 - Temporary label used to fetch the address of the potentially faulting instruction.
  */
.macro UNWIND_on_fault
    UNWIND_SECT
    .align 3
    .quad 10f // `f` refers to the next definition of this label
    .quad $0
    .text
    10:       // temporary label pointing at the actual potentially faulting instruction
.endmacro

/* Define a label pointing to the unwind section. We also need to define Lunwind_end below, after all
 * unwind entries have been defined. */
UNWIND_SECT
    .align 3
    Lunwind_start:

/* Define our ISRs */
.text
.align 3
.globl _trap_rdtsc_pf // PF (page fault) handler; implements recovery table support.
_trap_rdtsc_pf:
    /* Allocate stack space for %rax, %rdx, %rcx, our pass-through handler's address. */
    subq $32, %rsp

    /* Save the registers we'll be stomping */
    movq %rcx, 16(%rsp)
    movq %rax, 8(%rsp)
    movq %rdx, (%rsp)

    /* Fetch the address of the original ISR and save it on the stack */
    lookup_orig_isr 14, %rax
    movq %rax, 24(%rsp)

    /* Fetch the faulting rip from the stack (it's placed there at interrupt entry). */
    movq 40(%rsp), %rax

    /* Iterate our handler table */
    lea Lunwind_start(%rip), %rdx
    lea Lunwind_end(%rip), %rcx

Lunwind_lookup_iter:
    /* If we hit the end of the table, there's no match; we can delegate to the original handler */
    cmpq %rcx, %rdx
    je Lpassthrough_pf

    /* If the IP matches, delegate to the registered unwind handler */
    cmpq %rax, (%rdx)
    je Lpassthrough_unwind

    /* Otherwise, keep looking */
    addq $16, %rdx
    jmp Lunwind_lookup_iter

Lpassthrough_unwind:
    /* Replace the stack-saved ISR address with our unwinder. */
    movq 0x8(%rdx), %rax
    movq %rax, 16(%rsp)

    /* Fall through, letting the pass-through code invoke our unwinder */

Lpassthrough_pf:
    /* Restore rdx/rax. */
    popq %rcx
    popq %rdx
    popq %rax

    /* The original handler's address is now at the top of the stack; we can ret directly to it. */
    ret


.align 3
.globl _trap_rdtsc_gp // GP (general protection) handler; implements rdtsc emulation
_trap_rdtsc_gp:
    /* Allocate stack space for %rax, %rdx, and our pass-through handler's address. */
    subq $24, %rsp

    /* Save the registers we'll be stomping */
    movq %rax, 8(%rsp)
    movq %rdx, (%rsp)

    /* Fetch the address of the original ISR and save it on the stack */
    lookup_orig_isr 13, %rax
    movq %rax, 16(%rsp)

    /* Fetch the faulting rip from the stack (it's placed there at interrupt entry). */
    movq 32(%rsp), %rax

    /* Fetch the first 2 bytes at the faulting IP and check for rdtsc instruction. If a page fault occurs,
     * we'll simply delegate processing to the original ISR. */
    UNWIND_on_fault Lpassthrough_gp
    movw (%rax), %dx
    and $0xFFFF, %rdx
    cmp $0x310F, %rdx   // rdtsc == 0x310F
    je Lrdtsc

    /* If not rdtsc, it might be rdtscp */
    cmp $0xF901, %rdx   // first 2 bytes of rdtscp == 0xF901
    jne Lpassthrough_gp

    /* Fetch and check the last byte of possible rdtscp instruction */
    UNWIND_on_fault Lpassthrough_gp
    movb 2(%rax), %dl
    and $0xFF, %rdx
    cmp $0x0F, %rdx    // last byte of rdtscp == 0x0F
    jne Lpassthrough_gp

Lrdtscp:
    /* Populate ECX with IA32_TSC_AUX */
    movl $0x0, %ecx    // TODO - IA32_TSC_AUX MSR should be placed in ecx

    /* Move RIP past the faulting instruction. */
    addq $3, %rax       // rdtscp is a 3 byte instruction
    movq %rax, 32(%rsp)

    /* Delegate to the rdtsc path */
    jmp Lrdtsc_apply

Lrdtsc:
    /* Move RIP past the faulting instruction. */
    addq $2, %rax       // rdtsc is a 2 byte instruction
    movq %rax, 32(%rsp)

Lrdtsc_apply:
    /* Fetch the address of our counter */
    lea _traprdt_fake_counter(%rip), %rdx

    /* If the counter is 0 (uninitialized), try to populate it with real data; the goal is to avoid having the
     * counter move backwards relative to any measurements previously taken. */
    cmpq $0, (%rdx)
    jne Lrdtsc_finish

    /* Save our counter address in %rcx; we'll need three register operands for cmpxchgq*/
    pushq %rcx
    movq %rdx, %rcx

    /* Fetch the real TSC (stomping rdx + rax) */
    rdtsc

    /* Produce our 64-bit TSC representation */
    shlq $32, %rdx
    or %rax, %rdx

    /* Attempt atomic initialization of the counter. It doesn't matter if our initialization
     * succeeds; if it fails, it's because initialization succeeded elsewhere. */
    movq $0, %rax
    lock cmpxchgq %rdx, (%rcx)

    /* Restore %rdx (code below requires that this contain the address of our counter), and %rcx (previously pushed on stack) */
    movq %rcx, %rdx
    popq %rcx

    /* Fall through to the non-initialization path */

Lrdtsc_finish:
    /* Perform atomic increment of +100 */
    mov $100, %rax
	lock xaddq %rax, (%rdx)
    addq $100, %rax

    /* Populate the results */
    movq %rax, %rdx // high 32 bits
    movl %edx, %eax // low 32 bits
    shrq $32, %rdx

    /* Nothing to restore other than the stack pointer; rdtsc clobbers rdx and rax. */
    addq $32, %rsp // sizeof(rax, rdx, original handler) + error code pushed by CPU

    /* Return to the interrupted code */
    iretq

Lpassthrough_gp:
    /* Restore rdx/rax. */
    popq %rdx
    popq %rax

    /* The original handler's address is now at the top of the stack; we can ret directly to it. */
    ret

/* Define a label pointing to the end of our unwind section. */
UNWIND_SECT
    Lunwind_end: