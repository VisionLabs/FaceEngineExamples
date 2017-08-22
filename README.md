# LUNA SDK Examples
This repository contains examples code for VisionLabs LUNA SDK and is compatible
with SDK version 2.8.0 and newer.

**Please note, that while these examples are released under MIT license, the SDK itself is not.
Contact us via email (info@visionlabs.ru) for evaluation and/or licensing terms and conditions.**

Aside from examples itself, there are some supplementary materials you may find useful.
Look into *cmake/* folder for a CMake find script for the SDK. You are not forced to use
CMake but we advise for it.

Currently we support 64 bit Windows and Linux. On Windows everything should work with
Visual Studio 2015. On Linux we tested this code with GCC 4.8.5.
Other versions may work as well. Note, that the SDK is officially supported on RedHat
Linux families (RHEL, CentOS, Fedora).

## Build examples
From fsdk root.
```
$ mkdir build && cd build
$ cmake -DFSDK_ROOT=.. ../examples
```

## Run examples
Note: data folder must be at <fsdk_root>/data on Windows and on Linux.
```
$ build/example1/Example1 examples/images/Cameron_Diaz.ppm examples/images/Cameron_Diaz_2.ppm 0.7

$ build/example1/Example1 examples/images/Cameron_Diaz.ppm examples/images/Jennifer_Aniston.ppm 0.7

$ build/example2/Example2 examples/images/portrait.ppm

$ build/example3/Example3 examples/images/portrait.ppm

$ build/example4/Example4 examples/images/Cameron_Diaz.jpg
 
$ build/example6/Example6 examples/images/Cameron_Diaz.ppm examples/images/ examples/images_lists/list.txt 0.7

$ build/example7/Example7 examples/images/portrait.ppm

$ build/example8/Example8 examples/descriptors/Cameron_Diaz.xpk examples/descriptors/Cameron_Diaz_2.xpk 0.7
```

## Qt example
**Build with Qt example (from FSDK_ROOT/build):**
```
$ cmake -DFSDK_ROOT=.. -DWITH_QT_EXAMPLE=ON ../examples
```

If Qt is installed in non-system directory and can't be found by cmake automatically, you also need to specify path to Qt.
**Example:**
```
$ cmake -DFSDK_ROOT=.. -DWITH_QT_EXAMPLE=ON -DQt5Core_DIR=/usr/lib64/cmake/Qt5Core -DQt5Gui_DIR=/usr/lib64/cmake/Qt5Gui -DQt5_DIR=/usr/lib64/cmake/Qt5 ../examples
```

**Run Qt example:**
```
$ build/example5/Example5 examples/images/Jennifer_Aniston.jpg
```
