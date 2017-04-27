#include <FaceEngine.h>
#include <vlf/Log.h>

#include <iostream>
#include <vector>

#include "io_util.h"

int main(int argc, char *argv[])
{
    // Facial feature detection confidence threshold.
    // We use this value to reject bad face detections.
    const float confidenceThreshold = 0.25f;

    // Parse command line arguments.
    // We expect 1 of them:
    // 1) path to a image
    if (argc != 2) {
        std::cout << "USAGE: " << argv[0] << " <image>\n"
                " *image - path to image\n"
                << std::endl;
        return -1;
    }
    char *imagePath = argv[1];

    vlf::log::info("imagePath: \"%s\".", imagePath);

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

    // Create CNN warper.
    fsdk::IWarperPtr warper = fsdk::acquire(descriptorFactory->createWarper(fsdk::DT_CNN));
    if (!warper) {
        vlf::log::error("Failed to create face warper instance.");
        return -1;
    }

    // Create CNN descriptor extractor.
    fsdk::IDescriptorExtractorPtr descriptorExtractor = fsdk::acquire(descriptorFactory->createExtractor(fsdk::DT_CNN));
    if (!descriptorExtractor) {
        vlf::log::error("Failed to create face descriptor extractor instance.");
        return -1;
    }

    // Load image.
    fsdk::Image image;
    if (!image.loadFromPPM(imagePath)) {
        vlf::log::error("Failed to load image: \"%s\".", imagePath);
        return -1;
    }
    fsdk::Image imageBGR;
    image.convert(imageBGR, fsdk::Format::B8G8R8);

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
        vlf::log::error("Failed tor create face detection. Reason: %s.", detectorResult.what());
        return -1;
    }
    detectionsCount = detectorResult.getValue();
    vlf::log::info("Detections found: %d.", detectionsCount);

    // Feature set.
    fsdk::IFeatureSetPtr featureSet(nullptr);
    
    // Face descriptor.
    fsdk::IDescriptorPtr descriptor(nullptr);
    
    // Create descriptor batch.
    fsdk::IDescriptorBatchPtr descriptorBatch =
            fsdk::acquire(descriptorFactory->createDescriptorBatch(fsdk::DT_CNN, detectionsCount));
    if (!descriptorBatch) {
        vlf::log::error("Failed to create descriptor batch instance.");
        return -1;
    }

    // Loop through all the faces.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
	    fsdk::Detection &detection = detections[detectionIndex];

        // Estimate confidence score of face detection.
        if (detection.score < confidenceThreshold) {
            vlf::log::info("Face detection succeeded, but confidence score of detection is small.");
            continue;
        }

        // Create feature set.
        featureSet = fsdk::acquire(featureFactory->createFeatureSet(landmarks[detectionIndex], detection.score));
        if (!featureSet) {
            vlf::log::error("Failed to create face feature set instance.");
            return -1;
        }

        // Get warped face from detection.
        fsdk::Image warp;
        fsdk::Result<fsdk::FSDKError> warperResult = warper->warp(image, detection, featureSet, warp);
        if (warperResult.isError()) {
            vlf::log::error("Failed to create warp. Reason: %s.", warperResult.what());
            return -1;
        }

        // Save warped face.
        warp.saveAsPPM(("warp_" + std::to_string(detectionIndex) + ".ppm").c_str());
        
        // Create face descriptor.
        descriptor = fsdk::acquire(descriptorFactory->createDescriptor(fsdk::DT_CNN));
        if (!descriptor) {
            vlf::log::error("Failed to create face descriptor instance.");
            return -1;
        }

        // Extract face descriptor.
        fsdk::Result<fsdk::FSDKError> descriptorExtractorResult = descriptorExtractor->extract(
                imageBGR,
                detection,
                featureSet,
                descriptor
        );
        if (descriptorExtractorResult.isError()) {
            vlf::log::error("Failed to extract face descriptor. Reason: %s.", descriptorExtractorResult.what());
            return -1;
        }
        
        // Save face descriptor.
        std::vector<uint8_t> data;
        VectorArchive vectorArchive(data);
        if (!descriptor->save(&vectorArchive)) {
            vlf::log::error("Failed to save face descriptor to vector.");
        }
        if (!writeFile("descriptor_" + std::to_string(detectionIndex) + ".xpk", data)) {
            vlf::log::error("Failed to save face descritpor to file.");
            return -1;
        }
        
        // Add descriptor to descriptor barch.
        fsdk::Result<fsdk::DescriptorBatchError> descriptorBatchAddResult = descriptorBatch->add(descriptor);
        if (descriptorBatchAddResult.isError()) {
            vlf::log::error("Failed to add descriptor to descriptor batch.");
            return -1;
        }
    }
    
    // Save descriptor batch.
    std::vector<uint8_t> data;
    VectorArchive vectorArchive(data);
    if (!descriptorBatch->save(&vectorArchive)) {
        vlf::log::error("Failed to save descritpor batch to vector.");
        return -1;
    }
    if (!writeFile("descriptor_batch.xpk", data)) {
        vlf::log::error("Failed to save descritpor batch to file.");
        return -1;
    }

    return 0;
}
