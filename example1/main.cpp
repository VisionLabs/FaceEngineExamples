#include <FaceEngine.h>
#include <vlf/Log.h>

#include <iostream>

// Extract face descriptor.
fsdk::IDescriptorPtr extractDescriptor(
        fsdk::IDetectorPtr faceDetector,
        fsdk::IFeatureFactoryPtr featureFactory,
        fsdk::IFeatureDetectorPtr featureDetector,
        fsdk::IDescriptorFactoryPtr descriptorFactory,
        fsdk::IDescriptorExtractorPtr descriptorExtractor,
        const fsdk::Image &image
);

int main(int argc, char *argv[])
{
    // Parse command line arguments.
    // Arguments:
    // 1) path to a first image,
    // 2) path to a second image,
    // 3) matching threshold.
    // If matching score is above the threshold, then both images
    // belong to the same person, otherwise they belong to different persons.
    // Images should be in ppm format.
    if(argc != 4) {
        std::cout << "Usage: "<<  argv[0] << " <image1> <image2> <threshold>\n"
                " *image1 - path to first image\n"
                " *image2 - path to second image\n"
                " *threshold - similarity threshold in range (0..1]\n"
                << std::endl;
        return -1;
    }
    char *firstImagePath = argv[1];
    char *secondImagePath = argv[2];
    float threshold = (float)atof(argv[3]);

    vlf::log::info("firstImagePath: \"%s\".", firstImagePath);
    vlf::log::info("secondImagePath: \"%s\".", secondImagePath);
    vlf::log::info("threshold: %1.3f.", threshold);

    // Factory objects.
    // These set up various SDK components.
    // Note: SDK defines smart pointers for various object types named like IInterfacePtr.
    // Such objects implement reference counting to manage their life time. The smart pointers
    // will ensure that reference counting functions are called appropriately and the objects
    // are properly destroyed after use.
    fsdk::ISettingsProviderPtr config;                    // Config root SDK object.
    fsdk::IFaceEnginePtr faceEngine;                      // Root SDK object.
    fsdk::IDetectorFactoryPtr detectorFactory;            // Face detector factory.
    fsdk::IFeatureFactoryPtr featureFactory;              // Facial feature detector factory.
    fsdk::IDescriptorFactoryPtr descriptorFactory;        // Facial descriptor factory.

    // SDK components that we will use for facial recognition.
    fsdk::IDetectorPtr faceDetector;                      // Face detector.
    fsdk::IFeatureDetectorPtr featureDetector;            // Facial feature detector.
    fsdk::IDescriptorExtractorPtr descriptorExtractor;    // Facial descriptor extractor.
    fsdk::IDescriptorMatcherPtr descriptorMatcher;        // Descriptor matcher.

    // Create config FaceEngine root SDK object.
    config = fsdk::acquire(fsdk::createSettingsProvider("./data/faceengine.conf"));
    if (!config) {
        vlf::log::error("Failed to load face engine config instance.");
        return -1;
    }

    // Create FaceEngine root SDK object.
    faceEngine = fsdk::acquire(fsdk::createFaceEngine(fsdk::CFF_OMIT_SETTINGS));
    if (!faceEngine) {
        vlf::log::error("Failed to create face engine instance.");
        return -1;
    }
    faceEngine->setSettingsProvider(config);
    faceEngine->setDataDirectory("./data/");

    // Create detector factory.
    detectorFactory = fsdk::acquire(faceEngine->createDetectorFactory());
    if (!detectorFactory) {
        vlf::log::error("Failed to create face detector factory instance.");
        return -1;
    }

    // Create feature factory.
    featureFactory = fsdk::acquire(faceEngine->createFeatureFactory());
    if (!featureFactory) {
        vlf::log::error("Failed to create face feature factory instance.");
        return -1;
    }

    // Create descriptor factory.
    descriptorFactory = fsdk::acquire(faceEngine->createDescriptorFactory());
    if(!descriptorFactory) {
        vlf::log::error("Failed to create face descriptor factory instance.");
        return -1;
    }

    // Create DPM detector.
    faceDetector = fsdk::acquire(detectorFactory->createDetector(fsdk::ODT_DPM));
    if (!faceDetector) {
        vlf::log::error("Failed to create face detector instance.");
        return -1;
    }

    // Create VGG feature detector.
    featureDetector = fsdk::acquire(featureFactory->createDetector(fsdk::FET_VGG));
    if(!featureDetector) {
        vlf::log::error("Failed to create face feature detector instance.");
        return -1;
    }

    // Create CNN descriptor extractor.
    descriptorExtractor = fsdk::acquire(descriptorFactory->createExtractor(fsdk::DT_CNN));
    if (!descriptorExtractor) {
        vlf::log::error("Failed to create face descriptor extractor instance.");
        return -1;
    }

    // Create CNN descriptor matcher.
    descriptorMatcher = fsdk::acquire(descriptorFactory->createMatcher(fsdk::DT_CNN));
    if (!descriptorMatcher) {
        vlf::log::error("Failed to create face descriptor matcher instance.");
        return -1;
    }

    // Load images.
    fsdk::Image image1, image2;
    if (!image1.loadFromPPM(firstImagePath)) {
        vlf::log::error("Failed to load image: \"%s\".", firstImagePath);
        return -1;
    }
    if (!image2.loadFromPPM(secondImagePath)) {
        vlf::log::error("Failed to load image: \"%s\".", secondImagePath);
        return -1;
    }

    // Extract face descriptors.
    fsdk::IDescriptorPtr descriptor1 = extractDescriptor(
            faceDetector,
            featureFactory,
            featureDetector,
            descriptorFactory,
            descriptorExtractor,
            image1
    );
    fsdk::IDescriptorPtr descriptor2 = extractDescriptor(
            faceDetector,
            featureFactory,
            featureDetector,
            descriptorFactory,
            descriptorExtractor,
            image2
    );
    if (!descriptor1 || !descriptor2) {
        return -1;
    }

    // Descriptors similarity.
    float similarity;

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
    vlf::log::info("Derscriptors matched with score: %1.1f%%.", similarity * 100.f);

    // Check if similarity is above theshold.
    if (similarity > threshold)
        std::cout << "Images belong to one person." << std::endl;
    else
        std::cout << "Images belong to different persons." << std::endl;

   return 0;
}

fsdk::IDescriptorPtr extractDescriptor(
        fsdk::IDetectorPtr faceDetector,
        fsdk::IFeatureFactoryPtr featureFactory,
        fsdk::IFeatureDetectorPtr featureDetector,
        fsdk::IDescriptorFactoryPtr descriptorFactory,
        fsdk::IDescriptorExtractorPtr descriptorExtractor,
        const fsdk::Image &image
) {
    // Facial feature detection confidence threshold.
    const float confidenceThreshold = 0.25f;

    if (!image) {
        vlf::log::error("Request image is invalid.");
        return nullptr;
    }

    // Create color and grayscale images.
    fsdk::Image imageBGR;
    fsdk::Image imageR;
    image.convert(imageBGR, fsdk::Format::B8G8R8);
    image.convert(imageR, fsdk::Format::R8);
    if (!imageBGR) {
        vlf::log::error("Conversion to BGR has failed.");
        return nullptr;
    }
    if (!imageR) {
        vlf::log::error("Conversion to grayscale has failed.");
        return nullptr;
    }

    // Stage 1. Detect a face.
    vlf::log::info("Detecting faces.");

    // Detect no more than 10 faces in the image.
    enum { MaxDetections = 10 };
    fsdk::Detection detections[MaxDetections];
    int detectionsCount(MaxDetections);

    // Detect faces in the image.
    fsdk::ResultValue<fsdk::FSDKError, int> detectorResult = faceDetector->detect(
            imageR,
            imageR.getRect(),
            &detections[0],
            detectionsCount
    );
    if (detectorResult.isError()) {
        vlf::log::error("Failed to detect face detection. Reason: %s.", detectorResult.what());
        return nullptr;
    }
    detectionsCount = detectorResult.getValue();

    vlf::log::info("Found %d face(s).", detectionsCount);

    // Stage 2. Detect facial features set and compute a confidence score.
    vlf::log::info("Detecting facial features set.");

    fsdk::IFeatureSetPtr bestFeatureSet(nullptr);
    int bestDetectionIndex(0);

    // Create feature set.
    fsdk::IFeatureSetPtr featureSet = fsdk::acquire(featureFactory->createFeatureSet());
    if (!featureSet) {
        vlf::log::error("Failed to create face feature set instance.");
        return nullptr;
    }

    // Loop through all the faces and find one with the best score.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
        fsdk::Detection &detection = detections[detectionIndex];
        vlf::log::info("Detecting facial features (%d/%d).", (detectionIndex + 1), detectionsCount);

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

        // Choose the best feature set.
        if (!bestFeatureSet || featureSet->getConfidence() > bestFeatureSet->getConfidence()) {
            bestFeatureSet = featureSet;
            bestDetectionIndex = detectionIndex;
        }
    }

    // If not detect facial features or feature confidence score is too low, abort.
    if (!bestFeatureSet || bestFeatureSet->getConfidence() < confidenceThreshold) {
        vlf::log::info("Face detection succeeded, but no faces with good confidence found.");
        return nullptr;
    }
    vlf::log::info("Best face confidence is %0.3f.", bestFeatureSet->getConfidence());
    fsdk::Detection bestDetection = detections[bestDetectionIndex];

    // Stage 3. Create CNN face descriptor.
    vlf::log::info("Extracting descriptor.");

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

    return descriptor;
}
