# Build

## Requirements

* Xilinx Vivado version 2020.1
* GCC & G++

## Steps

* cd hw
* make create_project
* make csim
* make csynth
* make cosim

# Updating the Firmware Project

## Extracting Firmware Archive Files

The firmware update script requires firmware project files to be present in a directory named `devFramework`, present in the parent directory containing the `nxaccess-hls-framework` git repository.

```
- nxaccess-hls-framework/
	- hw/
	- scripts/
	- sw/
	- (...)
- top_xxx_delivery.tar.gz
- **devFramework/**
```

```bash
$ tar zxvf top_xxx_delivery.tar.gz
```

## Launching the Build Process

```bash
$ make build_firmware
```

This command, through the makefile ``build_firmware`` target, invokes a Python script located in *scripts/rebuild_firmware* which updates the firmware project licensing and configuration settings according to the contents of the *firmware_license.json* and *user_firmware_config.yaml* files.

## Checking Timing Closure

Timing results are displayed using the following command, which extracts values from the post-route physical optimization report file.

```bash
$ make show_timing_info
```

[w-hdl]: https://en.wikipedia.org/wiki/Hardware_description_language
[e-nxaccess]: https://www.enyx.com/nxaccess/