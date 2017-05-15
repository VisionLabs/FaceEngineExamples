# Example 1
## What it does
This example demonstrates how to detect a face on an image, how to extract biometric data
of that face (so called descriptor) and how to compare two descriptors. As the result of its work,
the example program will tell you whether people shown on two images are actually the same person or not.
It will also allow you to pick a threshold for such classification.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries
and tools; you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** already
(or at least have it somewhere nearby for reference) and know some core concepts,
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
SDK initialization. It is pretty straightforward
and self-explanatory, so we will not discuss it here.

### Stage 1. Face detection
This stage is implemented in ```extractDescriptor``` function.
At this stage we have an image. Presumably, there is a face somewhere in it and we would
like to find it. For that purpose, a face detector is used. We use it like so:
```C++
    // Detect no more than 10 faces in the image.
    enum { MaxDetections = 10 };
    fsdk::Detection detections[MaxDetections];
    int detectionsCount(MaxDetections);

    // Detect faces in the image.
    fsdk::Result<fsdk::FSDKError> detectorResult = faceDetector->detect(
            imageR,
            imageR.getRect(),
            &detections[0],
            &detectionsCount
    );
    if (detectorResult.isError()) {
        vlf::log::error("Failed to detect face detection. Reason: %s.", detectorResult.what());
        return nullptr;
    }
```
As the result we know whether we could detect faces (and how many of them) and what prevented us
from achieving that.

### Stage 2. Facial feature detection
This stage is implemented in ```extractDescriptor``` function.
We detect facial features (often called landmarks). We need these landmarks for two reasons:
1. they're required to extract a face descriptor (see below),
2. we can estimate landmarks alignment score and check if this face detection is good enough for us.

We use feature detector like so for all face detections:
```C++
    // Create feature set.
    fsdk::IFeatureSetPtr featureSet = fsdk::acquire(featureFactory->createFeatureSet());
    if (!featureSet) {
        vlf::log::error("Failed to create face feature set instance.");
        return nullptr;
    }

    // Detect feature set.
    fsdk::Result<fsdk::FSDKError> featureSetResult = featureDetector->detect(
            imageR,
            detection,
            featureSet
    );
    if (featureSetResult.isError()) {
        vlf::log::error("Failed to detect feature set. Reason: %s.", featureSetResult.what());
        return nullptr;
    }
```

### Stage 3. Descriptor extraction
This stage is implemented in ```extractDescriptor``` function.
When we have a reliable face detection, we need to extract some data from it that can be used
for comparison or *matching*. Such data is contained in descriptor object. A descriptor may be
extracted from an image using face detection and landmarks coordinates. Later, one or multiple
descriptors can be matched to determine face similarity.
```C++
    // Create CNN face descriptor.
    fsdk::IDescriptorPtr descriptor = fsdk::acquire(descriptorFactory->createDescriptor(fsdk::DT_CNN));
    if (!descriptor) {
        vlf::log::error("Failed to create face descrtiptor instance.");
        return nullptr;
    }

    // Extract face descriptor.
    // This is typically the most time-consuming task.
    fsdk::Result<fsdk::FSDKError> descriptorExtractorResult = descriptorExtractor->extract(
            imageBGR,
            bestDetection,
            bestFeatureSet,
            descriptor
    );
    if(descriptorExtractorResult.isError()) {
        vlf::log::error("Failed to extract face descriptor. Reason: %s.", descriptorExtractorResult.what());
        return nullptr;
    }
```

### Stage 4. Descriptor matching
This stage is pretty simple. We match two descriptors and see how similar they are.
For that we use a descriptor matcher object.
```C++
    // Match 2 descriptors.
    // Returns similarity in range (0..1],
    // where: 0 means totally different.
    //        1 means totally the same.
    fsdk::ResultValue<fsdk::FSDKError, fsdk::MatchingResult> descriptorMatcherResult =
            descriptorMatcher->match(descriptor1, descriptor2);
    if (descriptorMatcherResult.isError()) {
        vlf::log::error("Failed to match. Reason: %s.", descriptorMatcherResult.what());
        return -1;
    }

    similarity = descriptorMatcherResult.getValue().similarity;
```
*Similarity* score tells how similar these descriptors are. Its value is in (0..1] range.
Values near 1 tell us that the descriptors are very similar.

### Putting it all together
The ```main``` function just calls all the above in right order. Aside from that it parses
command line and loads images. After all stages finish it writes the result.
