# AsusMicMuteLED

Manual F9 microphone LED control for ASUS Vivobook X1605VA.

Version 0.1.0 froze this laptop when an on/off request was issued. Version 0.1.1 queues work outside the property setter and matches Linux's three-word WMI buffer. Status reads cached state. LED writes require a successful descriptor probe each boot. Hardware validation of 0.1.1 is pending.

After rebooting, run the descriptor probe first.

    sudo ./micled probe
    ./micled status

The probe verifies ATKD's WMI management GUID, NB method and instance count. It reads the firmware descriptor and performs no LED GPIO operation. After verifying the probe result, a manual write can be tested with on or off. Those writes still need runtime validation.

The tool reports request stages. A request timeout does not cancel a firmware operation. Kernel messages use the prefix AsusMicMuteLED.

    sudo ./micled on
    sudo ./micled off

This driver controls the LED independently of microphone audio. Automatic mute synchronization remains separate.

Build with bash Extras/AsusMicMuteLED/build.sh on macOS. The out directory contains the kext and command-line tool. Disable the Kernel/Add entry for rollback.

The interface follows this laptop's ATKD.WMNB descriptor and the Linux ASUS WMI implementation in drivers/platform/x86/asus-wmi.c. It uses device 0x00040017, DEVS method 0x53564544, instance zero and a reserved zero third word. The probe uses no DSTS call.
