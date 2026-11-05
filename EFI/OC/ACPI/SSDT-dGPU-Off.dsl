/*
 * Intel ACPI Component Architecture
 * AML/ASL+ Disassembler version 20260930 (64-bit version)
 * Copyright (c) 2000 - 2026 Intel Corporation
 * 
 * Disassembling to symbolic ASL+ operators
 *
 * Disassembly of SSDT-dGPU-Off.aml
 *
 * Original Table Header:
 *     Signature        "SSDT"
 *     Length           0x000000F9 (249)
 *     Revision         0x02
 *     Checksum         0x74
 *     OEM ID           "ACDT"
 *     OEM Table ID     "DGPU"
 *     OEM Revision     0x00000000 (0)
 *     Compiler ID      "INTL"
 *     Compiler Version 0x20260930 (539363632)
 */
DefinitionBlock ("", "SSDT", 2, "ACDT", "DGPU", 0x00000000)
{
    External (_SB_.PC00.PEG1.PEGP, DeviceObj)
    External (_SB_.PC00.PEG1.PEGP._OFF, MethodObj)    // 0 Arguments
    External (_SB_.PC00.PEG1.PEGP._PS3, MethodObj)    // 0 Arguments

    Device (RMD1)
    {
        Name (_HID, "RMD10000")  // _HID: Hardware ID
        Method (_INI, 0, NotSerialized)  // _INI: Initialize
        {
            If (CondRefOf (\_SB.PC00.PEG1.PEGP._OFF))
            {
                \_SB.PC00.PEG1.PEGP._OFF ()
            }
            ElseIf (CondRefOf (\_SB.PC00.PEG1.PEGP._PS3))
            {
                \_SB.PC00.PEG1.PEGP._PS3 ()
            }
        }
    }
}

