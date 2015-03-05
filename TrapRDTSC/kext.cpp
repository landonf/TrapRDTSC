#include <mach/mach_types.h>
#include <kern/thread.h>

#include "System.hpp"
#include "Logging.h"
#include "IDT.hpp"

using namespace traprdt;


/**
 * The table of CPUs (indexed by APIC/xAPIC local ID) used to track on which CPUs
 * `CR4` has been modified. Note that while this will continue to work on systems
 * using x2APIC, it /will/ obviously fail on systems with more than 256 processors.
 */
static uint8_t pending_cpus[256] = { 0 };

extern "C" {
    kern_return_t TrapRDTSC_start (kmod_info_t * ki, void *d);
    kern_return_t TrapRDTSC_stop (kmod_info_t *ki, void *d);
    
    /* GP ISR. */
    void trap_rdtsc_gp (void);
    
    /* Table of original ISRs. */
    uintptr_t traprdt_orig_isr[256] = { 0 };
    
    /** Atomically updated 64-bit counter */
    uint64_t traprdt_fake_counter = 0;
}



namespace traprdt {

    /* Configuration state passed to splat_cr4_thread */
    static struct splat_config {
        bool enable_tsd;
        volatile uint32_t pending_cpu_count;
    } _splat_config;

    /**
     * CR4 update thread. See explanation in run_on_all_CPUs() below.
     */
    static void splat_cr4_thread (void *ctx, wait_result_t) {
        splat_config *config = (splat_config *) ctx;
        bool enable_tsd = config->enable_tsd;
        uint8_t lapicID = 0xFF;

        /* Keep trying to splat CR4 until all CPUs have been splatted :-) */
        
        while (config->pending_cpu_count > 0) {
            auto cpu_done = CPU::disableInterrupts([&lapicID, enable_tsd]() {
                /* Determine CPU identifier */
                lapicID = CPU::current_lapicID();
                
                /* If this CPU has already been splatted, nothing else to do */
                if (pending_cpus[lapicID] == enable_tsd)
                    return false;
                
                /* Mark as processed */
                pending_cpus[lapicID] = enable_tsd;
                
                /* Fetch current value */
                uint64_t cr4 = 0;
                asm volatile (
                    "mov %%cr4, %0;"
                    : "=r" (cr4)
                );
                
                /* Set or unset TSD (time stamp disable) and then let interrupts resume */
                if (enable_tsd) {
                    cr4 |= 0x4;
                } else {
                    cr4 &= ~0x4;
                }
                asm volatile ("mov %0, %%cr4" :: "r" (cr4));
                return true;
            });
            
            if (cpu_done) {
                OSDecrementAtomic(&config->pending_cpu_count);
                break;
            }
        }
        
        TRLogDebug("TSD updated on CPU %d", (int) lapicID);
    }

    /*
     * There does not appear to be a convenient way to bind a kernel thread to a specific CPU, but
     * we need to execute on all logical CPUs to set CR4.
     *
     * To keep things simple, we take the approach of firing off cpu_count * 4 threads and having them spin
     * until they are able to execute their code on all CPUs.
     *
     * Alternatively, we might be able to hook IPI vector handling to dispatch our code on specific CPUs, but
     * dealing with xAPIC/x2APIC + IPI is far more complicated than brute-forcing the scheduler as we've
     * done here.
     */
    static void run_on_all_CPUs (size_t logical_cpu_count, bool enable_tsd) {
        /*
         * There does not appear to be a convenient way to bind a kernel thread to a specific CPU, but
         * we need to execute on all logical CPUs to set CR4.
         *
         * To keep things simple, we take the approach of firing off cpu_count+1 threads and having them spin
         * until they are able to execute their code on all CPUs.
         *
         * Alternatively, we might be able to hook IPI vector handling to dispatch our code on specific CPUs, but
         * dealing with xAPIC/x2APIC + IPI is far more complicated than brute-forcing the scheduler as we've
         * done here.
         */
        assert(logical_cpu_count <= UINT8_MAX);
        
        _splat_config.enable_tsd = enable_tsd,
        _splat_config.pending_cpu_count = (uint32_t) logical_cpu_count;

        for (uint32_t i = 0; i < (logical_cpu_count); i++) {
            thread_t thr;
            
            kern_return_t kt = kernel_thread_start(splat_cr4_thread, (void *) &_splat_config, &thr);
            if (kt != KERN_SUCCESS) {
                panic("TrapRDTSC: Error starting CR4 splat threads; system in unknown state");
            }
            thread_deallocate(thr);
        }
        
        /* Wait for completion */
        while (_splat_config.pending_cpu_count > 0);
    }

}

/* standard kext entry point */
kern_return_t TrapRDTSC_start (kmod_info_t *ki, void *d) {
    /*
     * Patch the IDT, inserting our own interrupt handlers.
     *
     * Note that while the IDT table is configured per-CPU, Mac OS X shares a common IDTR/IDT across all CPUs.
     */
    auto idt = CPU::sidt();
    CPU::disableWriteProtect([&idt](){
        /* Insert our RDTSC-emulating GP fault handler. */
        idt.patch(IDT::INT_GP, (uintptr_t) trap_rdtsc_gp, traprdt_orig_isr + IDT::INT_GP);
        return ftk::unit;
    });
    TRLogInfo("IDT patched successfully.");

    /* Fetch the number of CPUs */
    auto cpuCount = System.discoverCPUs() >> [&](const list<CPU> &cpus) {
        return cpus.size();
    };
    
    /* 'Splat' CR4 across all CPUs, disabling rdtsc */
    auto splatResult = cpuCount >> [](const size_t logical_cpu_count) {
        run_on_all_CPUs(logical_cpu_count, true);
        return unit;
    };
    
    return splatResult.match(
        [](const Error &error) {
            TRLogError("Unexpected failure: %s", error.description());
            TRLogError("System is in unknown state -- reboot immediately!");
            return KERN_FAILURE;
        },
        [](const Unit &) {
            return KERN_SUCCESS;
        }
    );
}

kern_return_t TrapRDTSC_stop (kmod_info_t *ki, void *d) {
    /* Fetch the number of CPUs */
    auto cpuCount = System.discoverCPUs() >> [&](const list<CPU> &cpus) {
        return cpus.size();
    };
    
    /* Unset the TSD flag in CR4 across all CPUs. */
    auto splatResult = cpuCount >> [](const size_t logical_cpu_count) {
        run_on_all_CPUs(logical_cpu_count, false);
        return unit;
    };
    
    /* Restore all patched ISRs */
    auto idt = CPU::sidt();
    CPU::disableWriteProtect([&idt](){
        for (size_t i = 0; i < sizeof(traprdt_orig_isr) / sizeof(traprdt_orig_isr[0]); i++) {
            if (traprdt_orig_isr[i] == 0x0)
                continue;
            
            idt.patch(i, traprdt_orig_isr[i], nullptr);
            traprdt_orig_isr[i] = 0x0;
        }
        return ftk::unit;
    });
    
    return splatResult.match(
        [](const Error &error) {
            TRLogError("Unexpected failure deregistering TrapRDTSC: %s", error.description());
            TRLogError("System is in unknown state -- reboot immediately!");
            return KERN_FAILURE;
        },
        [](const Unit &) {
            return KERN_SUCCESS;
        }
    );
}
