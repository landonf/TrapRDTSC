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
  * Fetch the address of the original ISR and save it in the designated destination.
  *
  * Arguments:
  *     $0 - Interrupt vector.
  *     $1 - Destination register.
  */
 .macro lookup_orig_isr
    movq ($0 * 8) + _traprdt_orig_isr(%rip), $1; // vector * sizeof(uintptr_t)
    movq %rax, $1;
 .endmacro

.text
.align 3
.globl _trap_rdtsc_gp
_trap_rdtsc_gp:
    /* Allocate stack space for %rax, %rdx, and our pass-through handler's address. */
    subq $24, %rsp;

    /* Save the registers we'll be stomping */
    movq %rax, 8(%rsp);
    movq %rdx, (%rsp);

    /* Fetch the address of the original ISR and save it on the stack */
    lookup_orig_isr 13, %rax;
    movq %rax, 16(%rsp);

    /* Fetch the faulting rip from the stack (it's placed there at interrupt entry) */
    movq 32(%rsp), %rax;

    /* Fetch the first 2 bytes at the faulting IP and check for rdtsc instruction. */
    // XXX TODO: We need to implement our own page fault handler for cases where reading RIP faults.
    movw (%rax), %dx;
    and $0xFFFF, %rdx;
    cmp $0x310F, %rdx;   // rdtsc == 0x310F
    je Lrdtsc;

    /* If not rdtsc, it might be rdtscp */
    cmp $0xF901, %rdx;   // first 2 bytes of rdtscp == 0xF901
    jne Lpassthrough;

    movb 2(%rax), %dl;  // fetch and check the last byte of possible rdtscp instruction
    and $0xFF, %rdx;
    cmp $0x0F, %rdx;    // last byte of rdtscp == 0x0F
    jne Lpassthrough;

Lrdtscp:
    /* Populate ECX with IA32_TSC_AUX */
    movl $0x0, %ecx;    // TODO - IA32_TSC_AUX MSR should be placed in ecx

    /* Move RIP past the faulting instruction. */
    addq $3, %rax       // rdtscp is a 3 byte instruction
    movq %rax, 32(%rsp)

    /* Delegate to the rdtsc path */
    jmp Lrdtsc_apply;

Lrdtsc:
    /* Move RIP past the faulting instruction. */
    addq $2, %rax       // rdtsc is a 2 byte instruction
    movq %rax, 32(%rsp)

Lrdtsc_apply:
    /* Fetch the address of our counter */
    lea _traprdt_fake_counter(%rip), %rdx

    /* Perform atomic increment of +100 */
    mov $100, %rax
	lock xaddq %rax, (%rdx)
    addq $100, %rax

    /* Populate the results */
    movq %rax, %rdx // high 32 bits
    movl %edx, %eax // low 32 bits
    shrq $32, %rdx

    /* Nothing to restore other than the stack pointer; rdtsc clobbers rdx and rax. */
    addq $32, %rsp // sizeof(rax, rdx, original handler) + error code inserted by kernel

    /* Return to the interrupted code */
    iretq;

Lpassthrough:
    /* Restore rdx/rax. */
    popq %rdx;
    popq %rax;

    /* The original handler's address is now at the top of the stack; we can ret directly to it. */
    ret;
