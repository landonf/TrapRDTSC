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

#include <mach/mach_types.h>
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

#include "CPU.hpp"
#include "Logging.h"
#include "Error.hpp"

#include "ftk/ftk.hpp"
#include "ttl/ttl.hpp"

namespace traprdt {
    
using namespace ttl;

constexpr class _System {
public:
    /**
     * Discover and return all logical CPUs.
     */
    Result<list<CPU>> discoverCPUs () const {
        using namespace ttl;
        
        /* Construct our service matching dictionary */
        OSDictionary    *matchingDictionary = OSDictionary::withCapacity(2);
        OSString        *processorType = OSString::withCString("processor");
        OSIterator      *iterator = NULL;
        
        /* Set up a matching dictionary on ACPI CPU nodes */
        IOService::serviceMatching("IOACPIPlatformDevice", matchingDictionary);
        IOService::propertyMatching(OSSymbol::withCString("device_type"), processorType, matchingDictionary);
        
        iterator = IOService::getMatchingServices(matchingDictionary);
        if (iterator == NULL) {
            return fail<list<CPU>>("CPU discovery failed, IOService::getMatchingServices() returned NULL");
        }

        OSObject *m;
        auto cpus = list<CPU>();
        while ((m = iterator->getNextObject()) != NULL) {
            IOACPIPlatformDevice *cpu = OSDynamicCast(IOACPIPlatformDevice, m);
            if (cpu == NULL) {
                TRLogInfo("Skipping non-IOACPIPlatformDevice CPU match");
                continue;
            }
            
            /* Extract the properties we care about */
            OSObject *property = cpu->getProperty("processor-lapic");
            if (property == NULL) {
                TRLogDebug("No APIC ID found for processor %s", cpu->getName());
                continue;
            }
            
            OSNumber *apicId = OSDynamicCast(OSNumber, property);
            if (apicId == NULL) {
                TRLogError("Skipping processor '%s' due to invalid process-lapic value; expected an OSNumber", cpu->getName());
                continue;
            }
            TRLogDebug("Found CPU \"%s\"", cpu->getName());
            
            /* Save the new CPU value. */
            uint32_t lapic_id = static_cast<uint32_t>(apicId->unsigned32BitValue());
            auto n = list<CPU>(CPU(lapic_id));
            cpus = concat(cpus, n);
        }
        
        /* Clean up */
        OSSafeRelease(matchingDictionary);
        OSSafeRelease(processorType);
        OSSafeRelease(iterator);
        
        return yield(cpus);
    }
private:
} System {};

}

