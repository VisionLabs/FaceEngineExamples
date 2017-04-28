#include <FaceEngine.h>
#include <vlf/Log.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <array>

// Helper function to load images.
bool loadImages(
        const char *imagesDirPath,
        const char *listPath,
        std::vector<std::string> &imagesNamesList,
        std::vector<fsdk::Image> &imagesList
);

// Extract face descriptor.
fsdk::IDescriptorPtr extractDescriptor(
        fsdk::IDetectorPtr detector,
        fsdk::IFeatureFactoryPtr featureFactory,
        fsdk::IDescriptorFactoryPtr descriptorFactory,
        fsdk::IDescriptorExtractorPtr descriptorExtractor,
        fsdk::Image &image
);

int main(int argc, char *argv[])
{
    // LSH (Local Sensitive Hashing) table interface.
    // The LSH tables allow to pick a given number of nearest neighbors, i.e. ones having the closest distance
	// to the user provided reference descritpor from a batch.
	// Each LSH table is tied to one descriptor batch. LSH tables are immutable objects
	// and therefore should be rebuilt if the corresponding batch is changed.
	// LSH table methods are not thread safe; users should create a table per thread
	// if parallel processing is required.

    // Number of required nearest neighbors.
    const int numberNearestNeighbors = 3;

	// Number of matching result.
	const int numberMatchingResult = 6;

    // Parse command line arguments.
    // Arguments:
    // 1) path to a image,
    // 2) path to a images directory,
    // 3) path to a images names list,
    // 4) matching threshold.
    // If matching score is above the threshold, then both images
    // belong to the same person, otherwise they belong to different persons.
    // Images should be in ppm format.
    if (argc != 5) {
        std::cout << "Usage: "<<  argv[0] << " <image> <imagesDir> <list> <threshold>\n"
                " *image - path to image\n"
                " *imagesDir - path to images directory\n"
                " *list - path to images names list\n"
                " *threshold - similarity threshold in range (0..1]\n"
                << std::endl;
        return -1;
    }
    char *imagePath = argv[1];
    char *imagesDirPath = argv[2];
    char *listPath = argv[3];
    float threshold = (float)atof(argv[4]);

    vlf::log::info("imagePath: \"%s\".", imagePath);
    vlf::log::info("imagesDirPath: \"%s\".", imagesDirPath);
    vlf::log::info("listPath: \"%s\".", listPath);
    vlf::log::info("threshold: %1.3f.", threshold);

    // Create config FaceEngine root SDK object.
    fsdk::ISettingsProviderPtr config;
    config = fsdk::acquire(fsdk::createSettingsProvider("./data/faceengine.conf"));
    if (!config) {
        vlf::log::error("Failed to load face engine config instance.");
        return -1;
    }

    // Create FaceEngine root SDK object.
    fsdk::IFaceEnginePtr faceEngine = fsdk::acquire(fsdk::createFaceEngine(fsdk::CFF_OMIT_SETTINGS));
    if (!faceEngine) {
        vlf::log::error("Failed to create face engine instance.");
        return -1;
    }
    faceEngine->setSettingsProvider(config);
    faceEngine->setDataDirectory("./data/");

    // Create detector factory.
    fsdk::IDetectorFactoryPtr detectorFactory = fsdk::acquire(faceEngine->createDetectorFactory());
    if (!detectorFactory) {
        vlf::log::error("Failed to create face detector factory instance.");
        return -1;
    }

    // Create MTCNN detector.
    fsdk::IDetectorPtr detector = fsdk::acquire(detectorFactory->createDetector(fsdk::ODT_MTCNN));
    if (!detector) {
        vlf::log::error("Failed to create face detector instance.");
        return -1;
    }

    // Create feature factory.
    fsdk::IFeatureFactoryPtr featureFactory = fsdk::acquire(faceEngine->createFeatureFactory());
    if (!featureFactory) {
        vlf::log::error("Failed to create face feature factory instance.");
        return -1;
    }

    // Create descriptor factory.
    fsdk::IDescriptorFactoryPtr descriptorFactory = fsdk::acquire(faceEngine->createDescriptorFactory());
    if (!descriptorFactory) {
        vlf::log::error("Failed to create face descriptor factory instance.");
        return -1;
    }

    // Create CNN descriptor extractor.
    fsdk::IDescriptorExtractorPtr descriptorExtractor =
            fsdk::acquire(descriptorFactory->createExtractor(fsdk::DT_CNN));
    if (!descriptorExtractor) {
        vlf::log::error("Failed to create face descriptor extractor instance.");
        return -1;
    }
    
    // Create CNN descriptor matcher.
    fsdk::IDescriptorMatcherPtr descriptorMatcher =
            fsdk::acquire(descriptorFactory->createMatcher(fsdk::DT_CNN));
    if (!descriptorMatcher) {
        vlf::log::error("Failed to create face descriptor matcher instance.");
        return -1;
    }

    // Load images.
    std::vector<std::string> imagesNamesList;
    std::vector<fsdk::Image> imagesList;
    if (!loadImages(imagesDirPath, listPath, imagesNamesList, imagesList)) {
        vlf::log::error("Failed to load images.");
        return -1;
    }

    vlf::log::info("Creating descriptor barch.");

    // Create CNN face descriptor batch.
    fsdk::IDescriptorBatchPtr descriptorBatch =
            fsdk::acquire(descriptorFactory->createDescriptorBatch(fsdk::DT_CNN, static_cast<int>(imagesList.size())));
    if (!descriptorBatch) {
        vlf::log::error("Failed to create face descriptor batch instance.");
        return -1;
    }

    // Extract faces descriptors.
    for (fsdk::Image &image : imagesList) {
        fsdk::IDescriptorPtr descriptor = extractDescriptor(
                detector,
                featureFactory,
                descriptorFactory,
                descriptorExtractor,
                image
        );
        if (!descriptor)
            return -1;
        fsdk::Result<fsdk::DescriptorBatchError> descriptorBatchAddResult =
                descriptorBatch->add(descriptor);
        if (descriptorBatchAddResult.isError()) {
            vlf::log::error("Failed to add descriptor to descriptor batch.");
            return -1;
        }
    }

    vlf::log::info("Creating LSH table.");

    // Create CNN LSH table.
    fsdk::ILSHTablePtr lsh =
            fsdk::acquire(descriptorFactory->createLSHTable(fsdk::DT_CNN, descriptorBatch.get()));
    if (!lsh) {
        vlf::log::error("Failed to create LSH table instance.");
        return -1;
    }

    // KNN index array.
    std::array<int, numberNearestNeighbors> nearestNeighbors;

    fsdk::Image image;
    if (!image.loadFromPPM(imagePath)) {
        vlf::log::error("Failed to load image: \"%s\".", imagePath);
        return false;
    }

    // Extract face descriptor.
    fsdk::IDescriptorPtr descriptor = extractDescriptor(
            detector,
            featureFactory,
            descriptorFactory,
            descriptorExtractor,
            image
    );
    if (!descriptor)
        return -1;

    // Get numberNearestNeighbors nearest neighbours.
    lsh->getKNearestNeighbours(descriptor, numberNearestNeighbors, &nearestNeighbors[0]);
    vlf::log::info("Name image: \"%s\", nearest neighbors: \"%s\", \"%s\", \"%s\".",
            imagePath,
            imagesNamesList[nearestNeighbors[0]].c_str(),
            imagesNamesList[nearestNeighbors[1]].c_str(),
            imagesNamesList[nearestNeighbors[2]].c_str()
    );

    // Match descriptor and descriptor batch.
    fsdk::MatchingResult matchingResult[numberMatchingResult];
    fsdk::Result<fsdk::FSDKError> descriptorMatcherResult =
            descriptorMatcher->match(
                    descriptor,
                    descriptorBatch,
                    &nearestNeighbors[0],
                    numberNearestNeighbors,
                    &matchingResult[0]
            );
    if (!descriptorMatcherResult) {
        vlf::log::error("Failed to match. Reason: %s.", descriptorMatcherResult.what());
        return -1;
    }

    std::ostringstream oss;

    for (size_t j = 0; j < imagesList.size(); ++j) {
        vlf::log::info("Images: \"%s\" and \"%s\" matched with score: %1.1f%%.",
                imagePath,
                imagesNamesList[j].c_str(),
                matchingResult[j].similarity * 100.f
        );

        oss << "Images: \"" << imagePath << "\" and \""
                << imagesNamesList[j] << "\" ";
        if (matchingResult[j].similarity > threshold)
            oss << "belong to one person." << std::endl;
        else
            oss << "belong to different persons." << std::endl;
    }

    std::cout << oss.str();

    return 0;
}

bool loadImages(
        const char *imagesDirPath,
        const char *listPath,
        std::vector<std::string> &imagesNamesList,
        std::vector<fsdk::Image> &imagesList
) {
    std::ifstream listFile(listPath);
    if (!listFile) {
        vlf::log::error("Failed to open file: %s.", listPath);
        return false;
    }
    std::string imageName;
    while (listFile >> imageName) {
        fsdk::Image image;
        std::string imagePath = std::string(imagesDirPath) + "/" + imageName;
        if (!image.loadFromPPM(imagePath.c_str())) {
            vlf::log::error("Failed to load image: \"%s\".", imagePath.c_str());
            return false;
        }
        imagesNamesList.push_back(imageName);
        imagesList.push_back(image);
    }
    listFile.close();

    return true;
}

fsdk::IDescriptorPtr extractDescriptor(
        fsdk::IDetectorPtr detector,
        fsdk::IFeatureFactoryPtr featureFactory,
        fsdk::IDescriptorFactoryPtr descriptorFactory,
        fsdk::IDescriptorExtractorPtr descriptorExtractor,
        fsdk::Image &image
) {
    // Facial feature detection confidence threshold.
    const float confidenceThreshold = 0.25f;

    if (!image) {
        vlf::log::error("Request image is invalid.");
        return nullptr;
    }

    // Create color image.
    fsdk::Image imageBGR;
    image.convert(imageBGR, fsdk::Format::B8G8R8);
    if (!imageBGR) {
        vlf::log::error("Conversion to BGR has failed.");
        return nullptr;
    }

    vlf::log::info("Detecting faces.");

    // Detect no more than 10 faces in the image.
    enum { MaxDetections = 10 };

    // Data used for MTCNN detection.
    fsdk::Detection detections[MaxDetections];
    int detectionsCount(MaxDetections);
    fsdk::IMTCNNDetector::Landmarks landmarks[MaxDetections];

    // Detect faces in the image.
    fsdk::ResultValue<fsdk::FSDKError, int> detectorResult =
            detector.as<fsdk::IMTCNNDetector>()->detect(
                    image,
                    image.getRect(),
                    &detections[0],
                    &landmarks[0],
                    detectionsCount
            );
    if (detectorResult.isError()) {
        vlf::log::error("Failed to create face detection. Reason: %s.", detectorResult.what());
        return nullptr;
    }
    detectionsCount = detectorResult.getValue();
    vlf::log::info("Found %d face(s).", detectionsCount);

    fsdk::IFeatureSetPtr bestFeatureSet(nullptr);
    int bestDetectionIndex(0);
    float bestDetectionScore(0.f);

    // Feature set.
    fsdk::IFeatureSetPtr featureSet(nullptr);

    // Loop through all the faces and find one with the best score.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
        fsdk::Detection &detection = detections[detectionIndex];

        // Estimate confidence score of face detection.
        if (detection.score < confidenceThreshold) {
            vlf::log::info("Face detection succeeded, but confidence score of detection is small.");
            continue;
        }

        // Choose the best feature set.
        if (bestFeatureSet && detection.score <= bestDetectionScore) {
            continue;
        }
        
        // Create feature set.
        featureSet = fsdk::acquire(featureFactory->createFeatureSet(landmarks[detectionIndex], detection.score));
        if (!featureSet) {
            vlf::log::error("Failed to create face feature set instance.");
            return nullptr;
        }

        bestDetectionIndex = detectionIndex;
        bestDetectionScore = detection.score;
        bestFeatureSet = featureSet;
    }

    // If not detect facial features or feature confidence score is too low, abort.
    if (!bestFeatureSet) {
        vlf::log::info("Face detection succeeded, but no faces with good confidence found.");
        return nullptr;
    }
    vlf::log::info("Best face confidence is %0.3f.", bestDetectionScore);
    fsdk::Detection bestDetection = detections[bestDetectionIndex];

    // Create a face descriptor.
    fsdk::IDescriptorPtr descriptor = fsdk::acquire(descriptorFactory->createDescriptor(fsdk::DT_CNN));
    if (!descriptor) {
        vlf::log::error("Failed to create face descrtiptor instance.");
        return nullptr;
    }

    vlf::log::info("Extracting descriptor.");

    // Extract face descriptor.
    // This is typically the most time consuming task.
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
