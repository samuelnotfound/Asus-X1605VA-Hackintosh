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
    IOACPIPlatformDevice *findATK();
    IOReturn probe();
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
    lock = IOLockAlloc();
    worker = thread_call_allocate(work, this);
    if (!lock || !worker)
        return false;
    setProperty("Version", "0.1.1");
    setProperty("Mode", "manual queued LED control; status is cached");
    publish("idle", true, kIOReturnSuccess, 0);
    registerService();
    return true;
}

void AsusMicMuteLED::stop(IOService *provider) {
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

IOACPIPlatformDevice *AsusMicMuteLED::findATK() {
    auto iterator = IORegistryIterator::iterateOver(
        gIOACPIPlane, kIORegistryIterateRecursively);
    if (!iterator)
        return nullptr;
    IOACPIPlatformDevice *found = nullptr;
    while (auto entry = iterator->getNextObject()) {
        auto candidate = OSDynamicCast(IOACPIPlatformDevice, entry);
        if (candidate && !strcmp(candidate->getName(), "ATKD")) {
            candidate->retain();
            found = candidate;
            break;
        }
    }
    iterator->release();
    return found;
}

IOReturn AsusMicMuteLED::probe() {
    if (!device)
        device = findATK();
    if (!device)
        return kIOReturnNotFound;
    if (device->validateObject("WMNB") != kIOReturnSuccess)
        return kIOReturnUnsupported;
    OSObject *object = nullptr;
    auto status = device->evaluateObject("_WDG", &object);
    auto data = OSDynamicCast(OSData, object);
    bool found = false;
    // 97845ED0-4E6D-11DE-8A39-0800200C9A66, method NB, instance 0.
    const UInt8 guid[16] = {0xD0,0x5E,0x84,0x97,0x6D,0x4E,0xDE,0x11,
                           0x8A,0x39,0x08,0x00,0x20,0x0C,0x9A,0x66};
    if (status == kIOReturnSuccess && data && data->getLength() % 20 == 0) {
        auto bytes = static_cast<const UInt8 *>(data->getBytesNoCopy());
        for (unsigned i = 0; i < data->getLength(); i += 20)
            if (!memcmp(bytes + i, guid, 16) &&
                bytes[i+16] == 'N' && bytes[i+17] == 'B' &&
                bytes[i+18] >= 1 && (bytes[i+19] & 2))
                found = true;
    }
    if (object)
        object->release();
    return found ? kIOReturnSuccess : kIOReturnUnsupported;
}

IOReturn AsusMicMuteLED::writeLED(UInt32 state, UInt32 *reply) {
    // Match Linux asus_wmi_evaluate_method3: device ID, state, reserved zero.
    const UInt32 args[3] = {0x00040017, state, 0};
    OSObject *parameters[3] = {
        OSNumber::withNumber(static_cast<unsigned long long>(0), 32),
        OSNumber::withNumber(0x53564544U, 32),
        OSData::withBytes(args, sizeof(args))
    };
    if (!parameters[0] || !parameters[1] || !parameters[2]) {
        for (auto parameter : parameters)
            if (parameter)
                parameter->release();
        return kIOReturnNoMemory;
    }
    auto status = device->evaluateInteger("WMNB", reply, parameters, 3);
    for (auto parameter : parameters)
        parameter->release();
    if (status == kIOReturnSuccess && *reply == 0xFFFFFFFEU)
        return kIOReturnUnsupported;
    return status;
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
        publish("probe-descriptor", false, kIOReturnSuccess, 0);
        result = probe();
        IOLockLock(lock);
        probeOK = result == kIOReturnSuccess;
        IOLockUnlock(lock);
    } else {
        publish("before-DEVS", false, kIOReturnSuccess, 0);
        result = writeLED(op == 1 ? 1 : 0, &reply);
    }
    publish(op == 0 ? "probe-complete" : "write-complete", true, result, reply);
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
