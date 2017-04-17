# Example 2
## What it does
This example demonstrates how to detect a face attributes on an image.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries and tools; you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already (or at least have it somewhere nearby for reference) and are familar with some core concepts, like memory management, object ownership and life-time control. This sample will not explain these aspects in detail.

## Example walkthrough
To get familiar with FSDK usage and common practices, please go through Example 1 first.

## How to run
./Example2_Attributes <some_image.ppm>

## Example output
```shell
Detections found: 2

Detection 0
Rect: x=181 y=158 w=99 h=100
Quality estimated
Quality: 0.766154
Complex attributes estimated
Gender: 0.0246919 (1 - man, 0 - woman)
Natural skin color: 0.989192 (1 - natural color of skin, 0 - not natural color of skin color)
Over exposed: 0.000160961 (1 - image is overexposed, 0 - image isn't overexposed)
Wear glasses: 0.000944621 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 25.4479 (in years)

Detection 1
Rect: x=259 y=428 w=100 h=100
Quality estimated
Quality: 0.664652
Complex attributes estimated
Gender: 0.99866 (1 - man, 0 - woman)
Natural skin color: 0.984361 (1 - natural color of skin, 0 - not natural color of skin color)
Over exposed: 0.000171973 (1 - image is overexposed, 0 - image isn't overexposed)
Wear glasses: 0.000216884 (1 - person wears glasses, 0 - person doesn't wear glasses)
Age: 23.7893 (in years)
```
