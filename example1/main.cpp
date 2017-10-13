#include <FaceEngine.h>

#include <iostream>

// Extract face descriptor.
fsdk::IDescriptorPtr extractDescriptor(
        fsdk::IFaceEnginePtr faceEngine,
        fsdk::IDetectorPtr faceDetector,
        fsdk::IDescriptorExtractorPtr descriptorExtractor,
        fsdk::Image &image
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
    if (argc != 4) {
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

    std::clog << "firstImagePath: \"" << firstImagePath <<"\"" << std::endl;
    std::clog << "secondImagePath: \"" << secondImagePath << "\"" << std::endl;
    std::clog << "threshold: " << threshold << std::endl;

    // Factory objects.
    // These set up various SDK components.
    // Note: SDK defines smart pointers for various object types named like IInterfacePtr.
    // Such objects implement reference counting to manage their life time. The smart pointers
    // will ensure that reference counting functions are called appropriately and the objects
    // are properly destroyed after use.
    fsdk::ISettingsProviderPtr config;                    // Config root SDK object.
    fsdk::IFaceEnginePtr faceEngine;                      // Root SDK object.

    // SDK components that we will use for facial recognition.
    fsdk::IDetectorPtr faceDetector;                      // Face detector.
    fsdk::IDescriptorExtractorPtr descriptorExtractor;    // Facial descriptor extractor.
    fsdk::IDescriptorMatcherPtr descriptorMatcher;        // Descriptor matcher.

    // Create config FaceEngine root SDK object.
    config = fsdk::acquire(fsdk::createSettingsProvider("./data/faceengine.conf"));
    if (!config) {
        std::cerr << "Failed to load face engine config instance." << std::endl;
        return -1;
    }

    // Create FaceEngine root SDK object.
    faceEngine = fsdk::acquire(fsdk::createFaceEngine());
    if (!faceEngine) {
        std::cerr << "Failed to create face engine instance." << std::endl;
        return -1;
    }
    faceEngine->setSettingsProvider(config);
    faceEngine->setDataDirectory("./data/");

    // Create MTCNN detector.
    faceDetector = fsdk::acquire(faceEngine->createDetector(fsdk::ODT_MTCNN));
    if (!faceDetector) {
        std::cerr << "Failed to create face detector instance." << std::endl;
        return -1;
    }

    // Create descriptor extractor.
    descriptorExtractor = fsdk::acquire(faceEngine->createExtractor());
    if (!descriptorExtractor) {
        std::cerr << "Failed to create face descriptor extractor instance." << std::endl;
        return -1;
    }

    // Create descriptor matcher.
    descriptorMatcher = fsdk::acquire(faceEngine->createMatcher());
    if (!descriptorMatcher) {
        std::cerr << "Failed to create face descriptor matcher instance." << std::endl;
        return -1;
    }

    // Load images.
    fsdk::Image image1, image2;
    if (!image1.loadFromPPM(firstImagePath)) {
        std::cerr << "Failed to load image: \"" << firstImagePath << "\"." << std::endl;
        return -1;
    }
    if (!image2.loadFromPPM(secondImagePath)) {
        std::cerr << "Failed to load image: \"" << secondImagePath << "\"" << std::endl;
        return -1;
    }

    // Extract face descriptors.
    fsdk::IDescriptorPtr descriptor1 = extractDescriptor(
            faceEngine,
            faceDetector,
            descriptorExtractor,
            image1
    );
    fsdk::IDescriptorPtr descriptor2 = extractDescriptor(
            faceEngine,
            faceDetector,
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
        std::cerr << "Failed to match. Reason: " << descriptorMatcherResult.what() << std::endl;
        return -1;
    }

    similarity = descriptorMatcherResult.getValue().similarity;
    std::clog << "Derscriptors matched with score: " << similarity * 100.f << "%" << std::endl;

    // Check if similarity is above theshold.
    if (similarity > threshold)
        std::cout << "Images belong to one person." << std::endl;
    else
        std::cout << "Images belong to different persons." << std::endl;

   return 0;
}

fsdk::IDescriptorPtr extractDescriptor(
        fsdk::IFaceEnginePtr faceEngine,
        fsdk::IDetectorPtr faceDetector,
        fsdk::IDescriptorExtractorPtr descriptorExtractor,
        fsdk::Image &image
) {
    // Facial feature detection confidence threshold.
    const float confidenceThreshold = 0.25f;

    if (!image) {
        std::cerr << "Request image is invalid." << std::endl;
        return nullptr;
    }

    // Stage 1. Detect a face.
    std::clog << "Detecting faces." << std::endl;

    // Detect no more than 10 faces in the image.
    enum { MaxDetections = 10 };

    // Data used for detection.
    fsdk::Detection detections[MaxDetections];
    int detectionsCount(MaxDetections);
    fsdk::IDetector::Landmarks5 landmarks5[MaxDetections];

    // Detect faces in the image.
    fsdk::ResultValue<fsdk::FSDKError, int> detectorResult = faceDetector->detect(
            image,
            image.getRect(),
            &detections[0],
            &landmarks5[0],
            detectionsCount
    );
    if (detectorResult.isError()) {
        std::cerr << "Failed to detect face detection. Reason: " << detectorResult.what() << std::endl;
        return nullptr;
    }
    detectionsCount = detectorResult.getValue();
    if (detectionsCount == 0) {
        std::clog << "Faces is not found." << std::endl;
        return nullptr;
    }
    std::clog << "Found " << detectionsCount << " face(s)." << std::endl;

    int bestDetectionIndex(-1);
    float bestScore(-1.f);
    // Loop through all the faces and find one with the best score.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
        fsdk::Detection &detection = detections[detectionIndex];
        std::clog << "Detecting facial features (" << detectionIndex + 1 << "/" << detectionsCount << std::endl;

        // Choose the best detection.
        if (detection.score > bestScore) {
            bestScore = detection.score;
            bestDetectionIndex = detectionIndex;
        }
    }

    // If detection confidence score is too low, abort.
    if (bestScore < confidenceThreshold) {
        std::clog << "Face detection succeeded, but no faces with good confidence found." << std::endl;
        return nullptr;
    }
    std::clog << "Best face confidence is " << bestScore << std::endl;

    // Stage 2. Create face descriptor.
    std::clog << "Extracting descriptor." << std::endl;

    // Create face descriptor.
    fsdk::IDescriptorPtr descriptor = fsdk::acquire(faceEngine->createDescriptor());
    if (!descriptor) {
        std::cerr << "Failed to create face descrtiptor instance." << std::endl;
        return nullptr;
    }

    // Extract face descriptor.
    // This is typically the most time-consuming task.
    fsdk::Result<fsdk::FSDKError> descriptorExtractorResult = descriptorExtractor->extract(
            image,
            detections[bestDetectionIndex],
            landmarks5[bestDetectionIndex],
            descriptor
    );
    if(descriptorExtractorResult.isError()) {
        std::cerr << "Failed to extract face descriptor. Reason: " << descriptorExtractorResult.what() << std::endl;
        return nullptr;
    }

    return descriptor;
}
