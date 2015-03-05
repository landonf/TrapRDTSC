TrapRDTSC
=========

TrapRDTSC traps and emulates the `rdtsc` (read time stamp counter) and `rdtscp` instructions
on Mac OS X/x86-64.

This is primarily useful when reverse engineering software that uses rdtsc to detect
external instrumentation by comparing rdtsc measurements and taking alternative/non-viable
execution paths when -- as is likely to occur when tracing with a debugger -- too much
time has passed.

In addition to rdtsc's use in malware [malware](https://www.f-secure.com/weblog/archives/00002067.html),
it is most notably also employed by Apple's iCloud service client code as a means to hinder reverse
engineering and ultimately re-implementations of the key exchange and client identification
mechanisms.

Implementation Notes
--------------------

TrapRDTSC works by:
    - setting the TSD (time stamp disable) bit in CR4, causing rdtsc to trigger
      a general protection fault outside of ring 0.
    - Patching the GP interrupt vector to detect and emulate rdtsc instructions.

TrapRDTSC currently uses an unprotected read to fetch the faulting instruction
from its GP interrupt handler, but does not provide page fault recovery mechanism. As
such, until we add page fault recovery handling, the kext can easily trigger a page
fault-induced panic.

Compatibility Notes
-------------------

The kext has only been tested on Mac OS X 10.9. Since the implementation has few
dependencies on the kernel, it should work on both earlier and later releases.
