# Description

This package contains a demonstration application of the nxAccess HLS Framework.

This framework based on our nxAccess market access solution allows software engineers 
to develop advanced algorithms to run directly on an FPGA.

For more information about the nxAccess solution please refer to our website: www.enyx.com/nxAccess

# Build

## Requirements

* Vivado 2018.3 tool suite
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
