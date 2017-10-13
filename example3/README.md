# Example 3
## What it does
This example demonstrates how to use the FreeImage library to processing images of different
formats by example the MTCNN detector and to estimate a face attributes, quality and eye on an image.

NOTE: In this example, the required dependencies for FreeImage are automatically installed.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries and tools;
you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already
(or at least have it somewhere nearby for reference) and are familiar with some core concepts,
like memory management, object ownership and life-time control. This sample will not explain
these aspects in detail.

More detailed information about the FreeImage library can be obtained by clicking on the link
http://freeimage.sourceforge.net/.

## Example walkthrough
To get familiar with FSDK usage and common practices, please go through Example 1 first.

## How to run
./Example3 <some_image>

## Example output
Warped images with faces.
```
Detection 1
Rect: x=277 y=426 w=73 h=94
Attribure estimate:
gender: 0.999705 (1 - man, 0 - woman)
wearGlasses: 0.000118364 (1 - person wears glasses, 0 - person doesn't wear glasses)
age: 17.7197 (in years)
Quality estimate:
light: 0.962603
dark: 0.974558
gray: 0.980648
blur: 0.955808
quality: 0.955808
Eye estimate:
left eye state: 2 (0 - close, 1 - open, 2 - noteye)
right eye state: 2 (0 - close, 1 - open, 2 - noteye)

Detection 2
Rect: x=203 y=159 w=63 h=89
Attribure estimate:
gender: 0.0053403 (1 - man, 0 - woman)
wearGlasses: 0.000911222 (1 - person wears glasses, 0 - person doesn't wear glasses)
age: 16.1504 (in years)
Quality estimate:
light: 0.964406
dark: 0.971644
gray: 0.981737
blur: 0.955808
quality: 0.955808
Eye estimate:
left eye state: 0 (0 - close, 1 - open, 2 - noteye)
right eye state: 2 (0 - close, 1 - open, 2 - noteye)
```
