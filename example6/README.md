# Example 6
## What it does
This example demonstrates how to create an LSH table from the batch descriptors,
use it to find the nearest neighbors with subsequent matches.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers,
libraries and tools; you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already
(or at least have it somewhere nearby for reference) and know some core concepts,
like memory management, object ownership and life-time control. This sample will not explain
these aspects in detail.

## Example walkthrough
To get familiar with FSDK usage and common practices, please go through Example 1 first.

## How to run
./Example6 <image.ppm> <imagesDir> <list> <threshold>

## Example output
```
Images: "images/Cameron_Diaz.ppm" and "Cameron_Diaz.ppm" belong to one person.
Images: "images/Cameron_Diaz.ppm" and "Cameron_Diaz_2.ppm" belong to one person.
Images: "images/Cameron_Diaz.ppm" and "Jason_Statham.ppm" belong to different persons.
Images: "images/Cameron_Diaz.ppm" and "Jason_Statham_2.ppm" belong to different persons.
Images: "images/Cameron_Diaz.ppm" and "Jennifer_Aniston.ppm" belong to different persons.
Images: "images/Cameron_Diaz.ppm" and "Jennifer_Aniston_2.ppm" belong to different persons.
```
