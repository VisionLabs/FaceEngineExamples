# Example 2
## What it does
This example demonstrates how to estimate a face quality on an image and to detect attributes on a face.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries and tools;
you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already (or at least have it
somewhere nearby for reference) and are familiar with some core concepts, like memory management,
object ownership and life-time control. This sample will not explain these aspects in detail.

## Example walkthrough
To get familiar with FSDK usage and common practices, please go through Example 1 first.

## How to run
./Example2 <some_image.ppm>

## Example output
Warped images with faces.
```
Detection 1
Rect: x=181 y=158 w=99 h=100
Quality estimated
Quality: 0.953474
Complex attributes estimated
Gender: 0.00851618 (1 - man, 0 - woman)
Wear glasses: 0.000833967 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 15.7716 (in years)

Detection 2
Rect: x=259 y=428 w=100 h=100
Quality estimated
Quality: 0.955808
Complex attributes estimated
Gender: 0.999883 (1 - man, 0 - woman)
Wear glasses: 7.95563e-05 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 17.647 (in years)
```
