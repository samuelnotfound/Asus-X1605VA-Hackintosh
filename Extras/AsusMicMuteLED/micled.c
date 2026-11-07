#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 2 || (strcmp(argv[1], "on") && strcmp(argv[1], "off") &&
                      strcmp(argv[1], "status"))) {
        fprintf(stderr, "Usage: sudo micled on|off|status\n");
        return 2;
    }
    io_service_t service = IOServiceGetMatchingService(
        kIOMainPortDefault, IOServiceMatching("AsusMicMuteLED"));
    if (!service) {
        fprintf(stderr, "AsusMicMuteLED.kext is not loaded.\n");
        return 1;
    }
    CFMutableDictionaryRef request = CFDictionaryCreateMutable(
        NULL, 0, &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!strcmp(argv[1], "status")) {
        CFDictionarySetValue(request, CFSTR("ReadMicMuteLED"), kCFBooleanTrue);
    } else {
        int state = !strcmp(argv[1], "on");
        CFNumberRef value = CFNumberCreate(NULL, kCFNumberIntType, &state);
        CFDictionarySetValue(request, CFSTR("MicMuteLED"), value);
        CFRelease(value);
    }
    kern_return_t result = IORegistryEntrySetCFProperties(service, request);
    CFRelease(request);
    if (result != KERN_SUCCESS) {
        fprintf(stderr, "LED request failed: 0x%08x\n", result);
        IOObjectRelease(service);
        return 1;
    }
    CFTypeRef value = IORegistryEntryCreateCFProperty(
        service, CFSTR("MicMuteLEDState"), NULL, 0);
    int state = -1;
    if (value && CFGetTypeID(value) == CFNumberGetTypeID())
        CFNumberGetValue(value, kCFNumberIntType, &state);
    if (value)
        CFRelease(value);
    IOObjectRelease(service);
    printf("Firmware reports F9 mic-mute LED %s\n",
           state == 1 ? "on" : state == 0 ? "off" : "unknown");
    return state < 0;
}
