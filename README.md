# LUNA SDK Examples
This repository contains example code for VisionLabs LUNA SDK and is compatible
with SDK version 1.0 and newer.

**Please note, that while these examples are released under MIT license, the SDK itself is not.
Contact us via email (info@visionlabs.ru) for evaluation and/or licensing terms and conditions.**

Aside from examples itself, there are some supplementary materials you may find useful.
Look into *cmake/* folder for a CMake find script for the SDK. Your are not forced to use
CMake but we advise for it.

Currently we support 64 bit Windows and Linux. On Windows everything should work with
Visual Studio 2015. On Linux we tested this code with GCC 4.8.5.
Other versions may work as well. Note, that the SDK is officially supported on RedHat
Linux families (RHEL, CentOS, Fedora).

## Build examples
From fsdk root.
```
$ mkdir build && cd build
$ cmake -DFSDK_ROOT=.. ../LUNASDKExamples
```

## Run examples
Note: data folder must be at <fsdk_root>/data on Windows and on Linux.
```
$ build/example1/Example1 LUNASDKExamples/images/Cameron_Diaz.ppm \
> LUNASDKExamples/images/Cameron_Diaz_2.ppm 0.7

$ build/example1/Example1 LUNASDKExamples/images/Cameron_Diaz.ppm \
> LUNASDKExamples/images/Jennifer_Aniston.ppm 0.7

$ build/example2/Example2 LUNASDKExamples/images/portrait.ppm

$ build/example3/Example3 LUNASDKExamples/images/portrait.ppm

$ build/example4/Example4 LUNASDKExamples/images/Cameron_Diaz.jpg

$ build/example5/Example5 LUNASDKExamples/images/Jennifer_Aniston.jpg

$ build/example6/Example6 LUNASDKExamples/images/Cameron_Diaz.ppm \
> LUNASDKExamples/images/ LUNASDKExamples/images_lists/list.txt 0.7

$ build/example3/Example7 LUNASDKExamples/images/portrait.ppm

$ build/example8/Example8 LUNASDKExamples/descriptors/Cameron_Diaz.xpk \
> LUNASDKExamples/descriptors/Cameron_Diaz_2.xpk 0.7
```
