# Example 2
## What it does
This example demonstrates how to estimate a face quality on an image and to detect attributes on an face.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries and tools;
you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already (or at least have it
somewhere nearby for reference) and are familar with some core concepts, like memory management,
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
Quality: 0.978872
Complex attributes estimated
Gender: 0.00665839 (1 - man, 0 - woman)
Natural skin color: 0.990255 (1 - natural color of skin, 0 - not natural color of skin color)
Over exposed: 0.000135384 (1 - image is overexposed, 0 - image isn't overexposed)
Wear glasses: 4.52165e-05 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 15.824 (in years)

Detection 2
Rect: x=259 y=428 w=100 h=100
Quality estimated
Quality: 0.982283
Complex attributes estimated
Gender: 0.999782 (1 - man, 0 - woman)
Natural skin color: 0.982964 (1 - natural color of skin, 0 - not natural color of skin color)
Over exposed: 0.000237707 (1 - image is overexposed, 0 - image isn't overexposed)
Wear glasses: 2.0116e-05 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 20.803 (in years)
```
