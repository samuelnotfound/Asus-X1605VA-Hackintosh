#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSData.h>
#include <kern/task.h>
#include <kern/thread_call.h>

class AsusMicMuteLED : public IOService {
    OSDeclareDefaultStructors(AsusMicMuteLED)
    IOLock *lock {nullptr};
    thread_call_t worker {nullptr};
    bool busy {false}, stopping {false}, probeOK {false};
    UInt32 operation {0};
    UInt64 requestID {0};
    IOACPIPlatformDevice *device {nullptr};
    static void work(thread_call_param_t owner, thread_call_param_t);
    void run();
    IOReturn writeLED(UInt32 state, UInt32 *reply);
    void publish(const char *stage, bool complete, IOReturn result, UInt32 reply);
public:
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    void free() override;
    IOReturn setProperties(OSObject *properties) override;
};
OSDefineMetaClassAndStructors(AsusMicMuteLED, IOService)

bool AsusMicMuteLED::start(IOService *provider) {
    if (!IOService::start(provider))
        return false;
    // Bind directly to the ATKD provider. Never walk the ACPI registry on request.
    device = OSDynamicCast(IOACPIPlatformDevice, provider);
    if (!device || strcmp(device->getName(), "ATKD")) {
        device = nullptr;
        return false;
    }
    device->retain();
    lock = IOLockAlloc();
    worker = thread_call_allocate(work, this);
    if (!lock || !worker)
        return false;
    setProperty("Version", "0.2.0");
    setProperty("Mode", "EXPERIMENTAL direct SGOV; cached provider check only");
    setProperty("ProviderBound", true);
    setProperty("FirmwareValidated", false);
    setProperty("GPIOPin", 0x09020007ULL, 32);
    publish("idle", true, kIOReturnSuccess, 0);
    registerService();
    return true;
}

void AsusMicMuteLED::stop(IOService *provider) {
    if (!lock || !worker) {
        IOService::stop(provider);
        return;
    }
    IOLockLock(lock);
    stopping = true;
    IOLockUnlock(lock);
    thread_call_cancel_wait(worker);
    IOService::stop(provider);
}

void AsusMicMuteLED::free() {
    if (worker)
        thread_call_free(worker);
    if (device)
        device->release();
    if (lock)
        IOLockFree(lock);
    IOService::free();
}

IOReturn AsusMicMuteLED::writeLED(UInt32 state, UInt32 *reply) {
    // X1605VA BIOS305 DEVS(0x40017, state) calls this exact GPIO helper.
    // Its return is void; evaluateObject must be used, not evaluateInteger.
    // No WMNB, _WDG, DSTS or registry traversal is performed here.
    OSObject *parameters[2] = {
        OSNumber::withNumber(0x09020007ULL, 32),
        OSNumber::withNumber(static_cast<unsigned long long>(state != 0), 32)
    };
    if (!parameters[0] || !parameters[1]) {
        for (auto parameter : parameters)
            if (parameter) parameter->release();
        return kIOReturnNoMemory;
    }
    auto result = device->evaluateObject("\\_SB.SGOV", nullptr, parameters, 2);
    for (auto parameter : parameters) parameter->release();
    *reply = 0; // SGOV has no return value; this is NOT GPIO readback.
    return result;
}

void AsusMicMuteLED::publish(const char *stage, bool complete,
                            IOReturn result, UInt32 reply) {
    auto dict = OSDictionary::withCapacity(5);
    auto id = OSNumber::withNumber(requestID, 64);
    auto status = OSNumber::withNumber(static_cast<UInt32>(result), 32);
    auto value = OSNumber::withNumber(reply, 32);
    auto step = OSString::withCString(stage);
    if (dict && id && status && value && step) {
        dict->setObject("RequestID", id);
        dict->setObject("Complete", complete ? kOSBooleanTrue : kOSBooleanFalse);
        dict->setObject("Result", status);
        dict->setObject("FirmwareReply", value);
        dict->setObject("Stage", step);
        setProperty("Status", dict);
    }
    if (id) id->release();
    if (status) status->release();
    if (value) value->release();
    if (step) step->release();
    if (dict) dict->release();
    IOLog("AsusMicMuteLED: request %llu %s result=0x%x reply=0x%x\n",
          requestID, stage, result, reply);
}

void AsusMicMuteLED::work(thread_call_param_t owner, thread_call_param_t) {
    static_cast<AsusMicMuteLED *>(owner)->run();
}

void AsusMicMuteLED::run() {
    IOLockLock(lock);
    auto op = operation;
    IOLockUnlock(lock);
    UInt32 reply = 0;
    IOReturn result;
    if (op == 0) {
        // A cached provider check only. No ACPI method is executed by probe.
        publish("provider-check", false, kIOReturnSuccess, 0);
        result = device ? kIOReturnSuccess : kIOReturnNotReady;
        IOLockLock(lock);
        probeOK = result == kIOReturnSuccess;
        IOLockUnlock(lock);
    } else {
        publish("before-SGOV", false, kIOReturnSuccess, 0);
        result = writeLED(op == 1 ? 1 : 0, &reply);
    }
    publish(op == 0 ? "provider-check-complete" : "write-complete-no-readback", true, result, reply);
    IOLockLock(lock);
    busy = false;
    IOLockUnlock(lock);
}

IOReturn AsusMicMuteLED::setProperties(OSObject *properties) {
    if (IOUserClient::clientHasPrivilege(current_task(),
            kIOClientPrivilegeAdministrator) != kIOReturnSuccess)
        return kIOReturnNotPrivileged;
    auto dict = OSDynamicCast(OSDictionary, properties);
    if (!dict || dict->getCount() != 2)
        return kIOReturnBadArgument;
    auto op = OSDynamicCast(OSNumber, dict->getObject("Operation"));
    auto id = OSDynamicCast(OSNumber, dict->getObject("RequestID"));
    if (!op || op->unsigned64BitValue() > 2 || !id ||
        id->unsigned64BitValue() == 0)
        return kIOReturnBadArgument;
    IOLockLock(lock);
    IOReturn result = kIOReturnSuccess;
    if (stopping)
        result = kIOReturnOffline;
    else if (busy)
        result = kIOReturnBusy;
    else if (op->unsigned32BitValue() != 0 && !probeOK)
        result = kIOReturnNotReady;
    else {
        operation = op->unsigned32BitValue();
        requestID = id->unsigned64BitValue();
        busy = true;
        thread_call_enter(worker);
    }
    IOLockUnlock(lock);
    return result;
}
