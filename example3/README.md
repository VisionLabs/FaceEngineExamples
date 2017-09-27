# Example 3
## What it does
This example demonstrates how to use the MTCNN detector and MTCNN feature set and
to estimate a face quality on an image and to detect attributes on a face.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries and tools;
you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already
(or at least have it somewhere nearby for reference) and are familiar with some core concepts,
like memory management, object ownership and life-time control. This sample will not explain
these aspects in detail.

## Example walkthrough
To get familiar with FSDK usage and common practices, please go through Example 1 first.

## How to run
./Example3 <some_image.ppm>

## Example output
Warped images with faces.
```
Detection 1
Rect: x=277 y=426 w=73 h=94
Quality estimated
Quality: 0.955808
Complex attributes estimated
Gender: 0.999705 (1 - man, 0 - woman)
Wear glasses: 0.000118364 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 17.7197 (in years)

Detection 2
Rect: x=203 y=159 w=63 h=89
Quality estimated
Quality: 0.955808
Complex attributes estimated
Gender: 0.0053403 (1 - man, 0 - woman)
Wear glasses: 0.000911222 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 16.1504 (in years)
```
