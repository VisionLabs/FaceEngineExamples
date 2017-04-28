# Example 4
## What it does
This example demonstrates how to use the FreeImage library to processing images of different
formats by example the MTCNN detector and MTCNN feature set and to estimate a face quality
on an image and to detect attributes on an face.

NOTE: In this example, the required dependencies for FreeImage are automatically installed.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries and tools;
you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already
(or at least have it somewhere nearby for reference) and are familar with some core concepts,
like memory management, object ownership and life-time control. This sample will not explain
these aspects in detail.

More detailed information about the FreeImage library can be obtained by clicking on the link
http://freeimage.sourceforge.net/.

## Example walkthrough
To get familiar with FSDK usage and common practices, please go through Example 1 first.

## How to run
./Example4 <some_image>

## Example output
Warped images with faces.
```
Detection 1
Rect: x=201 y=160 w=66 h=89
Quality estimated
Quality: 0.979267
Complex attributes estimated
Gender: 0.00671071 (1 - man, 0 - woman)
Natural skin color: 0.994203 (1 - natural color of skin, 0 - not natural color of skin color)
Over exposed: 0.000141749 (1 - image is overexposed, 0 - image isn't overexposed)
Wear glasses: 3.98217e-05 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 16.6324 (in years)

Detection 2
Rect: x=280 y=423 w=72 h=97
Quality estimated
Quality: 0.978351
Complex attributes estimated
Gender: 0.999703 (1 - man, 0 - woman)
Natural skin color: 0.992363 (1 - natural color of skin, 0 - not natural color of skin color)
Over exposed: 0.000166408 (1 - image is overexposed, 0 - image isn't overexposed)
Wear glasses: 2.47389e-05 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 21.8263 (in years)
```
