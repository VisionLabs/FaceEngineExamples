# Example 1
## What it does
This example demonstrates how to detect a face on an image, how to extract biometry data
of that face (so called descriptor) and how to compare two descriptors. As the result of it's work,
the example program will tell you whether people shown on two images are actually the same person or not.
It will also allow you to pick a threshold for such classification.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries
and tools; you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already
(or at least have it somewhere nearby for reference) and are familar with some core concepts,
like memory management, object ownership and life-time control. This sample will not explain
these aspects in detail.

The FaceEngine SDK is not about image/video loading, format decoding and such things.
It's about facial recognition. So this example will not cover topics like how to load an image.
There are no such functions built into the SDK either. For demonstration purposes we've
included some freeware PPM reader code into the sample. We picked PPM image format because
it is simple. You can read abut it here: https://en.wikipedia.org/wiki/Netpbm_format.
To prepare data for tests you may use such tools as ImageMagick, Gimp, XnView.

## Example walkthrough
We present a rather rich face processing pipeline. In fact, it is a bit too rich for this
sample, but it was made so intentionally to give you a clue of some real world usage scenarios.

### Stage 0. Preparations
SDK initialization is done in one function ```initFaceEngine```. It is pretty straightforward
and self-explanatory, so we will not discuss it here.

### Stage 1. Face detection
This stage is implemented in ```extractDescriptor``` function.
At this stage we have an image. Presumably, there is a face somewhere in it and we would
like to find it. For that purpose, a face detector is used. We use it like so:
```C++
   // We assume no more than 10 faces per image.
   enum { MaxDetections = 10 };

   Detection detections[MaxDetections];
   int detectionCount(MaxDetections);

   // Detect up to 10 faces.
   // detectionCount will store actual number of faces found.
   if(auto r = g_faceDetector->detect(imageR, imageR.getRect(), &detections[0], &detectionCount)) {
      if(detectionCount) {
        // Got at least one face...
      }
      else {
        // No faces found...
      }
    }
    else {
      // Some error ocurred. Find out what happened via r.what()...
    }
```
As the result we know whether we could detect faces (and how many of them) and what prevented us
from achieving that.

### Stage 2. Facial feature detection
This stage is implemented in ```extractDescriptor``` function.
We detect facial features (often called landmarks). We need these landmarks for two reasons:
1. they're required to extract a face descriptor (see below)
2. we can estimate landmarks alignment score and check if this face detection is good enough for us.

We use feature detector like so for all face detections:
```C++
    // Create a feature set.
    auto featureSet = acquire(g_featureFactory->createFeatureSet());

    // Now fill it with data...
    if(auto r = g_featureDetector->detect(imageR, detections[detectionIndex], featureSet)) {
       // Got a feature set...
    }
    else {
      // Well, that's not good...
    }
 }
```

### Stage 3. Descriptor extraction
This stage is implemented in ```extractDescriptor``` function.
When we have a reliable face detection, we need to extract some data from it that can be used
for comparison or *matching*. Such data is contained in descriptor object. A descriptor may be
extracted form an image using face detection and landmarks coordinates. Later, one or multiple
descriptors can be matched to determine face similarity.
```C++
    // Create a face descriptor.
    auto descriptor = acquire(g_descriptorFactory->createDescriptor(DT_DEFAULT));

    // Extract usable data.
    if(auto r = g_descriptorExtractor->extract(imageBGR, bestDetection, bestFeatureSet, descriptor)) {
        // Okay...
    }
    else {
        // Failed...
    }
```

### Stage 4. Descriptor matching
This stage is implemented in ```matchDescriptors``` function.
This stage is pretty simple. We match two descriptors and see how similar they are.
For that we use a descriptor matcher object.
```C++
   if(auto r = g_descriptorMatcher->match(first, second)) {
      // Okay, now get similarity.
      similarity = r.getValue().similarity;
   }
   else {
      // Could no match....
   }
```
*Similarity* score tells how similar these descriptors are. It's value is in (0..1] range.
Values near 1 tell us that the descriptors are very similar.

### Putting it all together
The ```main``` function just calls all of the above in rigth order. Aside from that it parses
command line and loads images. After all stages finish it writes the result.
