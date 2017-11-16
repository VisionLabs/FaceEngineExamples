#include <fsdk/FaceEngine.h>

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
        fsdk::IFaceEnginePtr faceEngine,
        fsdk::IDetectorPtr faceDetector,
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

    std::clog << "imagePath: \"" << imagePath << "\"" << std::endl;
    std::clog << "imagesDirPath: \"" << imagesDirPath << "\"" << std::endl;
    std::clog << "listPath: \"" << listPath << "\"" << std::endl;
    std::clog << "threshold: " << threshold << std::endl;

     // Create FaceEngine root SDK object.
    fsdk::IFaceEnginePtr faceEngine = fsdk::acquire(fsdk::createFaceEngine("./data", "./data/faceengine.conf"));
    if (!faceEngine) {
        std::cerr << "Failed to create face engine instance." << std::endl;
        return -1;
    }

    if (faceEngine->getFaceEngineEdition() != fsdk::FaceEngineEdition::CompliteEdition) {
        std::cerr << "FaceEngine SDK Frontend edition doesn't support face descriptors. Use FaceEngine SDK Complite edition" <<
            std::endl;
        return -1;
    }

    // Create MTCNN detector.
    fsdk::IDetectorPtr faceDetector = fsdk::acquire(faceEngine->createDetector(fsdk::ODT_MTCNN));
    if (!faceDetector) {
        std::cerr << "Failed to create face detector instance." << std::endl;
        return -1;
    }

    // Create descriptor extractor.
    fsdk::IDescriptorExtractorPtr descriptorExtractor =
            fsdk::acquire(faceEngine->createExtractor());
    if (!descriptorExtractor) {
        std::cerr << "Failed to create face descriptor extractor instance." << std::endl;
        return -1;
    }
    
    // Create descriptor matcher.
    fsdk::IDescriptorMatcherPtr descriptorMatcher =
            fsdk::acquire(faceEngine->createMatcher());
    if (!descriptorMatcher) {
        std::cerr << "Failed to create face descriptor matcher instance." << std::endl;
        return -1;
    }

    // Load images.
    std::vector<std::string> imagesNamesList;
    std::vector<fsdk::Image> imagesList;
    if (!loadImages(imagesDirPath, listPath, imagesNamesList, imagesList)) {
        std::cerr << "Failed to load images." << std::endl;
        return -1;
    }

    std::clog << "Creating descriptor barch." << std::endl;

    // Create face descriptor batch.
    fsdk::IDescriptorBatchPtr descriptorBatch =
            fsdk::acquire(faceEngine->createDescriptorBatch(static_cast<int>(imagesList.size())));
    if (!descriptorBatch) {
        std::cerr << "Failed to create face descriptor batch instance." << std::endl;
        return -1;
    }

    // Extract faces descriptors.
    for (fsdk::Image &image : imagesList) {
        fsdk::IDescriptorPtr descriptor = extractDescriptor(
                faceEngine,
                faceDetector,
                descriptorExtractor,
                image
        );
        if (!descriptor)
            return -1;
        fsdk::Result<fsdk::DescriptorBatchError> descriptorBatchAddResult =
                descriptorBatch->add(descriptor);
        if (descriptorBatchAddResult.isError()) {
            std::cerr << "Failed to add descriptor to descriptor batch." << std::endl;
            return -1;
        }
    }

    std::clog << "Creating LSH table." << std::endl;

    // Create LSH table.
    fsdk::ILSHTablePtr lsh =
            fsdk::acquire(faceEngine->createLSHTable(descriptorBatch.get()));
    if (!lsh) {
        std::cerr << "Failed to create LSH table instance." << std::endl;
        return -1;
    }

    // KNN index array.
    std::array<int, numberNearestNeighbors> nearestNeighbors;

    fsdk::Image image;
    if (!image.loadFromPPM(imagePath)) {
        std::cerr << "Failed to load image: \"" << imagePath << "\"" << std::endl;
        return -1;
    }

    // Extract face descriptor.
    fsdk::IDescriptorPtr descriptor = extractDescriptor(
            faceEngine,
            faceDetector,
            descriptorExtractor,
            image
    );
    if (!descriptor)
        return -1;

    // Get numberNearestNeighbors nearest neighbours.
    lsh->getKNearestNeighbours(descriptor, numberNearestNeighbors, &nearestNeighbors[0]);
    std::clog << "Name image: \"" << imagePath <<
        "\", nearest neighbors: \"" << imagesNamesList[nearestNeighbors[0]] <<
        "\", \"" << imagesNamesList[nearestNeighbors[1]] <<
        "\", \"" << imagesNamesList[nearestNeighbors[2]] << "\"" << std::endl;

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
        std::cerr << "Failed to match. Reason: " << descriptorMatcherResult.what() << std::endl;
        return -1;
    }

    std::ostringstream oss;

    for (size_t j = 0; j < imagesList.size(); ++j) {
        std::clog << "Images: \"" << imagePath <<
            "\" and \"" << imagesNamesList[j] << "\" matched with score: " <<
            matchingResult[j].similarity * 100.f << std::endl;

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
        std::cerr << "Failed to open file: " << listPath << std::endl;
        return false;
    }
    std::string imageName;
    while (listFile >> imageName) {
        fsdk::Image image;
        std::string imagePath = std::string(imagesDirPath) + "/" + imageName;
        if (!image.loadFromPPM(imagePath.c_str())) {
            std::cerr << "Failed to load image: \"" << imagePath << "\"" << std::endl;
            return false;
        }
        imagesNamesList.push_back(imageName);
        imagesList.push_back(image);
    }
    listFile.close();

    return true;
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

    // Create color image.
    fsdk::Image imageBGR;
    image.convert(imageBGR, fsdk::Format::B8G8R8);
    if (!imageBGR) {
        std::cerr << "Conversion to BGR has failed." << std::endl;
        return nullptr;
    }

     std::clog << "Detecting faces." << std::endl;

   // Detect no more than 10 faces in the image.
    enum { MaxDetections = 10 };

    // Data used for detection.
    fsdk::Detection detections[MaxDetections];
    int detectionsCount(MaxDetections);
    fsdk::Landmarks5 landmarks5[MaxDetections];

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
        std::clog << "Detecting facial features (" << detectionIndex + 1 << "/" << detectionsCount << ")" << std::endl;

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
    fsdk::ResultValue<FSDKError, float> descriptorExtractorResult = descriptorExtractor->extract(
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
