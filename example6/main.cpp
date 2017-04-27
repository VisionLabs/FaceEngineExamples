#include <FaceEngine.h>
#include <vlf/Log.h>

#include <iostream>
#include <fstream>
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

    // Facial feature detection confidence threshold.
    // Bad face detection score threshold.
    const float confidenceThreshold = 0.25f;

    // Parse command line arguments.
    // We expect 5 of them:
    // 1) path to a first images directory to match
    // 2) path to a second images directory to match
    // 3) path to a first images names list
    // 4) path to a second images names list
    // 5) matching threshold
    // If matching score is above the threshold, then both images
    // are of the same person, else different persons.
    if (argc != 6) {
        std::cout << "Usage: "<<  argv[0] << "<imagesPath1> <imagesPath2> <list1> <list2> <threshold>\n"
                " *imagesPath1 - path to first images directory\n"
                " *imagesPath1 - path to second images directory\n"
                " *list1 - path to first images names list\n"
                " *list2 - path to second images names list\n"
                " *threshold - similarity threshold in range (0..1]\n"
                << std::endl;
        return -1;
    }
    char *firstImagesDirPath = argv[1];
    char *secondImagesDirPath = argv[2];
    char *firstListPath = argv[3];
    char *secondListPath = argv[4];
    float threshold = (float)atof(argv[5]);

    vlf::log::info("firstImagesDirPath: \"%s\".", firstImagesDirPath);
    vlf::log::info("secondImagesDirPath: \"%s\".", secondImagesDirPath);
    vlf::log::info("firstListPath: \"%s\".", firstListPath);
    vlf::log::info("secondListPath: \"%s\".", secondListPath);
    vlf::log::info("threshold: %1.3f.", threshold);

    // Create config FaceEngine root SDK object.
    fsdk::ISettingsProviderPtr config;
    config = fsdk::acquire(fsdk::createSettingsProvider("./data/faceengine.conf"));
    if (!config) {
        vlf::log::error("Failed to load face engine config.");
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
    std::vector<std::string> firstImagesNamesList;
    std::vector<std::string> secondImagesNamesList;
    std::vector<fsdk::Image> firstImagesList;
    std::vector<fsdk::Image> secondImagesList;
    if (
            !loadImages(firstImagesDirPath, firstListPath, firstImagesNamesList, firstImagesList) ||
            !loadImages(secondImagesDirPath, secondListPath, secondImagesNamesList, secondImagesList)
    ) {
        vlf::log::error("Failed to load images.");
        return -1;
    }

    // Extract a face descriptor for matching.
    auto extractDescriptor = [&](fsdk::Image &image) -> fsdk::IDescriptorPtr {
        if (!image) {
            vlf::log::error("Request image is invalid.");
            return nullptr;
        }

        fsdk::Image imageBGR;
        image.convert(imageBGR, fsdk::Format::B8G8R8);
        if (!imageBGR) {
            vlf::log::error("Conversion to BGR has failed.");
            return nullptr;
        }


        vlf::log::info("Detecting faces.");

        // We assume no more than 10 faces per image.
        enum { MaxDetections = 10 };

        // Data used for MTCNN detection.
	    fsdk::Detection detections[MaxDetections];
	    int detectionsCount(MaxDetections);
	    fsdk::IMTCNNDetector::Landmarks landmarks[MaxDetections];

        // Detect faces on the photo.
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
        vlf::log::info("Detections found: %d.", detectionsCount);

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

        // If we could not detect facial features OR feature confidence score is too low, abort.
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

        // This is typically the most time consuming task so we would like to do it only when sure
        // that our image is good enough for recognition purposes.
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
    };

    vlf::log::info("Creating descriptor barch for list: \"%s\".", secondListPath);

    // Create CNN face descriptor batch.
    fsdk::IDescriptorBatchPtr secondDescriptorBatch =
            fsdk::acquire(descriptorFactory->createDescriptorBatch(fsdk::DT_CNN, secondImagesList.size()));

    // Extract face descriptors.
    for (fsdk::Image &image : secondImagesList) {
        fsdk::IDescriptorPtr descriptor = extractDescriptor(image);
        if (!descriptor)
            return -1;
        fsdk::Result<fsdk::DescriptorBatchError> descriptorBatchAddResult =
                secondDescriptorBatch->add(descriptor);
        if (descriptorBatchAddResult.isError()) {
            vlf::log::error("Failed to add descriptor to descriptor batch.");
            return -1;
        }
    }

    vlf::log::info("Creating LSH table.");

    // Create CNN LSH table.
    fsdk::ILSHTablePtr lsh =
            fsdk::acquire(descriptorFactory->createLSHTable(fsdk::DT_CNN, secondDescriptorBatch.get()));
    if (!lsh) {
        vlf::log::error("Failed to create LSH table instance.");
        return -1;
    }

    // KNN index array.
    std::array<int, numberNearestNeighbors> nearestNeighbors;

    for (int i = 0; i < firstImagesList.size(); ++i) {
        // Extract face descriptor.
        fsdk::IDescriptorPtr descriptor = extractDescriptor(firstImagesList[i]);
        if (!descriptor)
            return -1;

        // Get numberNearestNeighbors nearest neighbours.
        lsh->getKNearestNeighbours(descriptor, numberNearestNeighbors, &nearestNeighbors[0]);
        vlf::log::info("Name image: \"%s\", nearest neighbors: \"%s\", \"%s\", \"%s\".",
                firstImagesNamesList[i].c_str(),
                secondImagesNamesList[nearestNeighbors[0]].c_str(),
                secondImagesNamesList[nearestNeighbors[1]].c_str(),
                secondImagesNamesList[nearestNeighbors[2]].c_str()
        );

        // Match descriptor and descriptor batch.
        fsdk::MatchingResult matchingResult[secondImagesList.size()];
        fsdk::Result<fsdk::FSDKError> descriptorMatcherResult =
                descriptorMatcher->match(
                        descriptor,
                        secondDescriptorBatch,
                        &nearestNeighbors[0],
                        numberNearestNeighbors,
                        &matchingResult[0]
                );
        if (!descriptorMatcherResult) {
            vlf::log::error("Failed to match. Reason: %s.", descriptorMatcherResult.what());
            return -1;
        }

        for (size_t j = 0; j < 3; ++j) {
            vlf::log::info("Images: \"%s\" and \"%s\" matched with score: %1.1f%%.",
                    firstImagesNamesList[i].c_str(),
                    secondImagesNamesList[nearestNeighbors[j]].c_str(),
                    matchingResult[nearestNeighbors[j]].similarity * 100.f
            );

            std::cout << "Images: \"" << firstImagesNamesList[i] << "\" and \""
                    << secondImagesNamesList[nearestNeighbors[j]] << "\" ";
            if (matchingResult[nearestNeighbors[j]].similarity > threshold)
                std::cout << "belong to one person." << std::endl;
            else
                std::cout << "belong to different persons." << std::endl;
        }
    }

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
