#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSData.h>
#include <kern/task.h>

class AsusMicMuteLED : public IOService {
    OSDeclareDefaultStructors(AsusMicMuteLED)
    IOLock *lock {nullptr};
    IOACPIPlatformDevice *findATK();
    IOReturn evaluate(IOACPIPlatformDevice *device, UInt32 method,
                      UInt32 state, UInt32 *result);
public:
    bool start(IOService *provider) override;
    void free() override;
    IOReturn setProperties(OSObject *properties) override;
};
OSDefineMetaClassAndStructors(AsusMicMuteLED, IOService)

bool AsusMicMuteLED::start(IOService *provider) {
    if (!IOService::start(provider))
        return false;
    lock = IOLockAlloc();
    if (!lock)
        return false;
    setProperty("Version", "0.1.0");
    setProperty("Mode", "manual mic-mute LED control");
    registerService();
    return true;
}

void AsusMicMuteLED::free() {
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
        auto device = OSDynamicCast(IOACPIPlatformDevice, entry);
        if (device && !strcmp(device->getName(), "ATKD") &&
            device->validateObject("WMNB") == kIOReturnSuccess) {
            device->retain();
            found = device;
            break;
        }
    }
    iterator->release();
    return found;
}

IOReturn AsusMicMuteLED::evaluate(IOACPIPlatformDevice *device,
                                 UInt32 method, UInt32 state,
                                 UInt32 *result) {
    const UInt32 args[2] = {0x00040017, state};
    OSObject *parameters[3] = {
        OSNumber::withNumber(0, 32),
        OSNumber::withNumber(method, 32),
        OSData::withBytes(args, sizeof(args))
    };
    if (!parameters[0] || !parameters[1] || !parameters[2]) {
        for (auto parameter : parameters)
            if (parameter)
                parameter->release();
        return kIOReturnNoMemory;
    }
    auto status = device->evaluateInteger("WMNB", result, parameters, 3);
    for (auto parameter : parameters)
        parameter->release();
    return status;
}

IOReturn AsusMicMuteLED::setProperties(OSObject *properties) {
    if (IOUserClient::clientHasPrivilege(current_task(),
            kIOClientPrivilegeAdministrator) != kIOReturnSuccess)
        return kIOReturnNotPrivileged;
    auto dict = OSDynamicCast(OSDictionary, properties);
    if (!dict || dict->getCount() != 1)
        return kIOReturnBadArgument;
    auto number = OSDynamicCast(OSNumber, dict->getObject("MicMuteLED"));
    bool readOnly = dict->getObject("ReadMicMuteLED") == kOSBooleanTrue;
    if (!readOnly && (!number || number->unsigned64BitValue() > 1))
        return kIOReturnBadArgument;

    IOLockLock(lock);
    auto device = findATK();
    IOReturn status = kIOReturnNotFound;
    UInt32 value = 0;
    if (device) {
        // Query presence before accepting a write.
        status = evaluate(device, 0x53545344, 0, &value); // DSTS
        if (status == kIOReturnSuccess && !(value & 0x00010000))
            status = kIOReturnUnsupported;
        if (status == kIOReturnSuccess && !readOnly) {
            status = evaluate(device, 0x53564544,
                              number->unsigned32BitValue(), &value); // DEVS
            if (status == kIOReturnSuccess && value != 1)
                status = kIOReturnError;
            if (status == kIOReturnSuccess)
                status = evaluate(device, 0x53545344, 0, &value);
        }
        if (status == kIOReturnSuccess && (value & 0x00010000)) {
            setProperty("MicMuteLEDState", value & 1, 32);
            setProperty("FirmwareStatus", value, 32);
        } else if (status == kIOReturnSuccess) {
            status = kIOReturnUnsupported;
        }
        device->release();
    }
    setProperty("LastResult", static_cast<UInt32>(status), 32);
    IOLockUnlock(lock);
    return status;
}
