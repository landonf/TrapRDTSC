TrapRDTSC
=========

TrapRDTSC traps and emulates the `rdtsc` (read time stamp counter) and `rdtscp` instructions
on Mac OS X/x86-64.

This is primarily useful when reverse engineering software that uses rdtsc to detect
external instrumentation.

In addition to use in [malware](https://www.f-secure.com/weblog/archives/00002067.html),
`rdtsc` is employed by Mac OS X's iCloud/Spotlight/iMessage/apsd/... client implementations
as a means to hinder reverse engineering and ultimately re-implementations of the key
exchange and client identification mechanisms.

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
