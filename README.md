# FaceEngine Examples
This repository contains example code for VisionLabs FaceEngine SDK and is compatible with SDK version 1.0 and newer.

**Please note, that while these examples are released under MIT license, the SDK itself is not. Contact us via email (info@visionlabs.ru) for evaluation and/or licensing terms and conditions.**

Aside from examples itself, there are some supplementary materials you may find useful. Look into *cmake/* folder for a CMake find script for the SDK. Your are not forced to use CMake but we advise for it.

Currently we support 64 bit Windows and Linux. On Windows everything should work with Visual Studio 2012, 2013 and 2014. On Linux we tested this code with GCC 4.9.2 and CLang 3.5.0. Other versions may work as well. Note, that the SDK is officially supported on RedHat Linux families (RHEL, CentOS, Fedora).

## Build examples

From fsdk root.
```
$ mkdir build && cd build
$ cmake -DFSDK_ROOT=.. ../examples
```

## Run examples

Note: data folder must be at <fsdk_root>/data on Windows and /opt/visionlabs/data on Linux

```
$ build/example1/Example1 examples/portrait1.ppm portrait1.ppm 0.7
$ build/example1/Example1 examplex/portrait1.ppm portrait2.ppm 0.7
$ build/example2/Example2 examples/portrait.ppm
```
