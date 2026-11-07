#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static long long number(CFDictionaryRef dict, CFStringRef key) {
    CFTypeRef value = CFDictionaryGetValue(dict, key);
    long long result = 0;
    if (value && CFGetTypeID(value) == CFNumberGetTypeID())
        CFNumberGetValue(value, kCFNumberLongLongType, &result);
    return result;
}

static CFDictionaryRef readStatus(io_service_t service) {
    CFTypeRef value = IORegistryEntryCreateCFProperty(service, CFSTR("Status"), NULL, 0);
    if (value && CFGetTypeID(value) == CFDictionaryGetTypeID())
        return (CFDictionaryRef)value;
    if (value) CFRelease(value);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2 || (strcmp(argv[1],"status") && strcmp(argv[1],"check") &&
                      strcmp(argv[1],"on") && strcmp(argv[1],"off"))) {
        fprintf(stderr,"Usage: micled status | sudo micled check|on|off\n");
        return 2;
    }
    io_service_t service = IOServiceGetMatchingService(
        kIOMasterPortDefault, IOServiceMatching("AsusMicMuteLED"));
    if (!service) {
        fprintf(stderr,"AsusMicMuteLED.kext is not loaded.\n");
        return 1;
    }
    // Never send this client's commands to the known-freezing 0.1.x driver.
    if (strcmp(argv[1], "status")) {
        CFTypeRef version = IORegistryEntryCreateCFProperty(service, CFSTR("Version"), NULL, 0);
        int compatible = version && CFGetTypeID(version) == CFStringGetTypeID() &&
            CFEqual(version, CFSTR("0.2.0"));
        if (version) CFRelease(version);
        if (!compatible) {
            fprintf(stderr, "Refusing: loaded LED driver is not 0.2.0. Reboot with the candidate first.\n");
            IOObjectRelease(service);
            return 1;
        }
    }
    int exitCode = 1;
    long long requestID = 0;
    if (strcmp(argv[1],"status")) {
        int op = !strcmp(argv[1],"check") ? 0 : !strcmp(argv[1],"on") ? 1 : 2;
        requestID = ((long long)getpid() << 32) | arc4random();
        CFNumberRef id = CFNumberCreate(NULL,kCFNumberLongLongType,&requestID);
        CFNumberRef action = CFNumberCreate(NULL,kCFNumberIntType,&op);
        const void *keys[] = {CFSTR("RequestID"),CFSTR("Operation")};
        const void *values[] = {id,action};
        CFDictionaryRef params = CFDictionaryCreate(NULL,keys,values,2,
            &kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
        kern_return_t result = IORegistryEntrySetCFProperties(service,params);
        CFRelease(params); CFRelease(id); CFRelease(action);
        if (result != KERN_SUCCESS) {
            fprintf(stderr,"Request rejected: 0x%08x. Run check before on/off.\n",result);
            IOObjectRelease(service);
            return 1;
        }
    }
    char previous[128] = "";
    for (int i=0; i<200; ++i) {
        CFDictionaryRef status = readStatus(service);
        if (status) {
            long long seenID = number(status,CFSTR("RequestID"));
            CFTypeRef step = CFDictionaryGetValue(status,CFSTR("Stage"));
            char stage[128] = "unknown";
            if (step && CFGetTypeID(step)==CFStringGetTypeID())
                CFStringGetCString(step,stage,sizeof(stage),kCFStringEncodingUTF8);
            if (!requestID || seenID==requestID) {
                if (strcmp(stage,previous)) {
                    printf("%s (result 0x%08x, firmware reply 0x%08x)\n",stage,
                        (unsigned)number(status,CFSTR("Result")),
                        (unsigned)number(status,CFSTR("FirmwareReply")));
                    fflush(stdout);
                    strcpy(previous,stage);
                }
                if (!requestID || CFDictionaryGetValue(status,CFSTR("Complete"))==kCFBooleanTrue) {
                    exitCode=number(status,CFSTR("Result")) ? 1 : 0;
                    CFRelease(status);
                    break;
                }
            }
            CFRelease(status);
        }
        usleep(100000);
    }
    if (exitCode && requestID)
        fprintf(stderr,"Request failed or timed out. A timeout does not cancel a firmware call.\n");
    IOObjectRelease(service);
    return exitCode;
}
