#include <FaceEngine.h>

#include <iostream>
#include <vector>

#include "io_util.h"

int main(int argc, char *argv[])
{
    // Facial feature detection confidence threshold.
    const float confidenceThreshold = 0.25f;

    // Parse command line arguments.
    // Arguments:
    // 1) path to a first image.
    // Image should be in ppm format.
    if (argc != 2) {
        std::cout << "USAGE: " << argv[0] << " <image>\n"
                " *image - path to image\n"
                << std::endl;
        return -1;
    }
    char *imagePath = argv[1];

    std::clog << "imagePath: \"" << imagePath << "\"" << std::endl;

     // Create FaceEngine root SDK object.
    fsdk::IFaceEnginePtr faceEngine = fsdk::acquire(fsdk::createFaceEngine("./data", "./data/faceengine.conf"));
    if (!faceEngine) {
        std::cerr << "Failed to create face engine instance." << std::endl;
        return -1;
    }
    
    if (faceEngine->getFaceEngineEdition() != fsdk::FaceEngineEdition::CompliteEdition) {
        std::cerr << "FaceEngine SDK Frontend edition doesn't support face descriptors. Use FaceEngine SDK Complite edition" << std::endl;
        return -1;
    }

    // Create MTCNN detector.
    fsdk::IDetectorPtr faceDetector = fsdk::acquire(faceEngine->createDetector(fsdk::ODT_MTCNN));
    if (!faceDetector) {
        std::cerr << "Failed to create face detector instance." << std::endl;
        return -1;
    }

    // Create warper.
    fsdk::IWarperPtr warper = fsdk::acquire(faceEngine->createWarper());
    if (!warper) {
        std::cerr << "Failed to create face warper instance." << std::endl;
        return -1;
    }

    // Create descriptor extractor.
    fsdk::IDescriptorExtractorPtr descriptorExtractor = fsdk::acquire(faceEngine->createExtractor());
    if (!descriptorExtractor) {
        std::cerr << "Failed to create face descriptor extractor instance." << std::endl;
        return -1;
    }

    // Load image.
    fsdk::Image image;
    if (!image.loadFromPPM(imagePath)) {
        std::cerr << "Failed to load image: \"" << imagePath << "\"" << std::endl;
        return -1;
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
        return -1;
    }
    detectionsCount = detectorResult.getValue();
    if (detectionsCount == 0) {
        std::clog << "Faces is not found." << std::endl;
        return 0;
    }
    std::clog << "Found " << detectionsCount << " face(s)." << std::endl;
    
    // Face descriptor.
    fsdk::IDescriptorPtr descriptor(nullptr);
    
    // Create face descriptor batch.
    fsdk::IDescriptorBatchPtr descriptorBatch =
            fsdk::acquire(faceEngine->createDescriptorBatch(detectionsCount));
    if (!descriptorBatch) {
        std::cerr << "Failed to create face descriptor batch instance." << std::endl;
        return -1;
    }

    // Loop through all the faces.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
	    // Estimate confidence score of face detection.
        if (detections[detectionIndex].score < confidenceThreshold) {
            std::clog << "Face detection succeeded, but confidence score of detection is small." << std::endl;
            continue;
        }

        // Get warped face from detection.
        fsdk::Transformation transformation;
        fsdk::Image warp;
        transformation = warper->createTransformation(detections[detectionIndex], landmarks5[detectionIndex]);
        fsdk::Result<fsdk::FSDKError> warperResult = warper->warp(image, transformation, warp);
        if (warperResult.isError()) {
            std::cerr << "Failed to create warped face. Reason: " << warperResult.what() << std::endl;
            return -1;
        }

        // Save warped face.
        warp.saveAsPPM(("warp_" + std::to_string(detectionIndex) + ".ppm").c_str());
        
        // Create face descriptor.
        fsdk::IDescriptorPtr descriptor = fsdk::acquire(faceEngine->createDescriptor());
        if (!descriptor) {
            std::cerr << "Failed to create face descrtiptor instance." << std::endl;
            return -1;
        }

        // Extract face descriptor.
        // This is typically the most time-consuming task.
        fsdk::Result<fsdk::FSDKError> descriptorExtractorResult = descriptorExtractor->extract(
                image,
                detections[detectionIndex],
                landmarks5[detectionIndex],
                descriptor
        );
        if(descriptorExtractorResult.isError()) {
            std::cerr << "Failed to extract face descriptor. Reason: " << descriptorExtractorResult.what() << std::endl;
            return -1;
        }

        std::clog << "Saving descriptor (" << (detectionIndex + 1) << "/" << detectionsCount << ")" << std::endl;

        // Save face descriptor.
        std::vector<uint8_t> data;
        VectorArchive vectorArchive(data);
        if (!descriptor->save(&vectorArchive)) {
            std::cerr << "Failed to save face descriptor to vector." << std::endl;
        }
        if (!writeFile("descriptor_" + std::to_string(detectionIndex) + ".xpk", data)) {
            std::cerr << "Failed to save face descritpor to file." << std::endl;
            return -1;
        }

        std::clog << "Adding descriptor to descriptor barch (" <<
            (detectionIndex + 1) << "/" << detectionsCount << ")" << std::endl;

        // Add descriptor to descriptor barch.
        fsdk::Result<fsdk::DescriptorBatchError> descriptorBatchAddResult = descriptorBatch->add(descriptor);
        if (descriptorBatchAddResult.isError()) {
            std::cerr << "Failed to add descriptor to descriptor batch." << std::endl;
            return -1;
        }
    }

    std::clog << "Saving descriptor barch." << std::endl;

    // Save descriptor batch.
    std::vector<uint8_t> data;
    VectorArchive vectorArchive(data);
    if (!descriptorBatch->save(&vectorArchive)) {
        std::cerr << "Failed to save descritpor batch to vector." << std::endl;
        return -1;
    }
    if (!writeFile("descriptor_batch.xpk", data)) {
        std::cerr << "Failed to save descritpor batch to file." << std::endl;
        return -1;
    }

    return 0;
}
