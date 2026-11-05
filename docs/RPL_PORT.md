# Raptor Lake-P microphone port assessment

## X1605VA hardware

The saved Windows hardware inventory identifies PCI `00:1f.3` as Intel `8086:51CA` and the HDA codec as Realtek ALC256 (`10EC:0256`, codec subsystem `1043:1743`). Linux maps `51CA` to `HDA_RPL_P_0` and selects `rpl_desc` with `tgl_chip_info` (cAVS 2.5). Its default IPC3 firmware is `intel/sof/sof-rpl.ri`. The descriptor also supports IPC4 with a separate firmware directory and topology. [PCI IDs](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/include/linux/pci_ids.h), [RPL descriptor](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/pci-tgl.c), [DSP profile](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/tgl.c).

The saved 753-byte NHLT table passes its ACPI checksum. It has two SSP endpoints for Bluetooth (`8086:AE30`, virtual bus 2, render and capture) and zero PDM capture endpoints. Windows associates the active **Microphone Array** endpoint with Realtek(R) Audio and codec hardware ID `INTELAUDIO\FUNC_01&VEN_10EC&DEV_0256&SUBSYS_10431743`. The Intel bus enumerator is compatible with an HDA codec behind the SST controller. Together these observations point to the codec path and provide no positive evidence of a DSP PDM microphone. [NHLT endpoint definitions](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/include/acpi/actbl2.h).

## Comet Lake versus Raptor Lake-P

| Area | Comet Lake reference | Raptor Lake-P 51CA |
| --- | --- | --- |
| DSP profile and power | `cnl_chip_info`, cAVS 1.8. Linux starts core 0 and can manage cores 0–3. | `tgl_chip_info`, cAVS 2.5. Linux starts and manages core 0. |
| DSP BAR and reset | PCI BAR4. `ADSPCS` at `0x04` with reset, stall, power request and power acknowledgment at shifts 0, 8, 16 and 24. | Same BAR and register definitions. Limit core operations to the RPL host-managed mask. |
| ROM and code load | ROM status at `0x80000` and error at `0x80004`. Common `cl_dsp_init` handshake and HDA playback DMA loader. | Same ROM offsets and loader family. Linux checks ROM init and then `FW_ENTERED` (`0x5`) separately. |
| IPC | LatSOF uses IPC3 and embeds `sof-cml.ri`. | Linux defaults to IPC3 with `sof-rpl.ri`. Both profiles use CNL-style request `0xd0`, acknowledgment `0xd4` and control `0xe8`. IPC4 is a separate protocol, even with shared doorbell registers. |
| DMIC and topology | The verified CML laptop has DSP PDM mics. LatSOF carries prebuilt topology IPC messages for that pipeline. | No DSP PDM endpoint in the ASUS NHLT capture. No DMIC topology is justified for this unit. The generic SOF topology and its channel-count variants are separate files. |

Sources for the comparison are the Linux [CML descriptor](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/pci-cnl.c), [CNL DSP profile](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/cnl.c), [register definitions](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/hda.h), [loader](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/hda-loader.c), [power/reset](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/hda-dsp.c), [IPC](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/hda-ipc.c) and [topology selection](https://github.com/thesofproject/linux/blob/39db9fb3f0ef096d85793401b97da89f36ef7564/sound/soc/sof/intel/hda.c). The upstream [LatSOF porting guide](https://github.com/pokipoi/LatSOF-Audio/blob/main/docs/ADAPTING.md) and [firmware and topology distribution](https://github.com/thesofproject/sof-bin) describe the inputs to a port.

## Decision

Stop before the ROM handshake. The controller supports SOF, but the available evidence points to the Realtek capture path for this microphone. AppleALC 1.9.8 layout 5 labels its input Mic and routes it through codec node `0x1b`. The saved Windows Realtek settings include `Pin12` and `Pin21` entries. Those entries are clues. Confirm the internal mic pin and ADC route with an ALC256 codec widget dump before changing the layout. Preserve the working speaker route during capture tests. [AppleALC layout 5](https://github.com/acidanthera/AppleALC/blob/1.9.8/Resources/ALC256/layout5.xml), [layout 5 path map](https://github.com/acidanthera/AppleALC/blob/1.9.8/Resources/ALC256/Platforms5.xml).

The saved pre-Bluetooth OpenCore config has `DisableIoMapper=true`. Check the booted EFI before any future DSP test. If a later hardware capture establishes a DSP PDM route, keep `latsof-auto=0` and `hdaInit=0`, preserve AppleHDA's CORB/RIRB ownership, and run one bring-up stage per boot.
