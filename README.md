# nxAccess HLS Framework

This repository contains the **[nxAccess][e-nxaccess] HLS Framework** source code.

Using this framework, software engineers are able to implement advanced trading strategies on Enyx's FPGA board, taking full-advantage of hardware acceleration with direct market access.

Please visit [enyx.com/nxaccess][e-nxaccess] for more information.

## Quick Start

### Building the HLS project

```shell
cd nxaccess-hls-framework/hw
make create_project
make csim
make csynth
make cosim
```

> 📝 **Note**:
> 
> Please refer to the [`hw/README.md`](hw/README.md) document for software requirements; build process details as well as as troubleshooting.

### Generating a FPGA Firmware File

FPGA-based hardware accelerators must be configured using a FPGA bitstream file.

```shell
cd nxaccess-hls-framework/hw
make build_firmware
```

> ❕ **Important**:
> 
> This step requires an **Enyx FPGA devFramework** project. Please contact us at [enyx.com/contact][e-contact] for further information.

### Flashing the Firmware File on the FPGA board

Please refer to the *[Upgrade the FPGA with the latest firmware][um-fw-upgrade]* section in the nxAccess user manual.

### Building the Software

Once [you installed][um-install-sdk] the nxAccess packages, you can follow the standard CMake-based flow : 

```shell
$ mkdir build 
$ cd build 
$ cmake .. 
$ make 
```

You end up with packages that can be installed, and the following binaries:

 * `enyx-oe-hwstrat-hls-demo` that is a demonstration of the hls core configuration.

[w-hdl]: https://en.wikipedia.org/wiki/Hardware_description_language
[e-nxaccess]: https://www.enyx.com/nxaccess/
[e-contact]: https://www.enyx.com/contact/
[um-fw-upgrade]: https://www.enyx.com/documentation/nxaccess/guides/nxaccess-user-manual/index.html#upgrade-the-fpga-with-the-latest-firmware
[um-install-sdk]: https://www.enyx.com/documentation/nxaccess/guides/nxaccess-user-manual/index.html#install-the-software-api-sdk
