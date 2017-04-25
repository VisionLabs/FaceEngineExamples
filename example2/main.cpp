#include <FaceEngine.h>
#include <vlf/Log.h>

#include <iostream>

int main(int argc, char *argv[])
{
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

    // Config FaceEngine root SDK object.
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

    // Create DPM detector.
    fsdk::IDetectorFactoryPtr detectorFactory = fsdk::acquire(faceEngine->createDetectorFactory());
    if (!detectorFactory) {
        vlf::log::error("Failed to create face detector factory instance.");
        return -1;
    }
    fsdk::IDetectorPtr detector = fsdk::acquire(detectorFactory->createDetector(fsdk::ODT_DPM));
    if (!detector) {
        vlf::log::error("Failed to create face detector instance.");
        return -1;
    }

    // Create feature detector factory.
    fsdk::IFeatureFactoryPtr featureFactory = fsdk::acquire(faceEngine->createFeatureFactory());
    if (!featureFactory) {
        vlf::log::error("Failed to create face feature factory instance.");
        return -1;
    }
    fsdk::IFeatureDetectorPtr featureDetector = fsdk::acquire(featureFactory->createDetector(fsdk::FT_VGG));
    if (!featureDetector) {
        vlf::log::error("Failed to create face featrure detector instance.");
        return -1;
    }
    fsdk::IFeatureSetPtr featureSet = fsdk::acquire(featureFactory->createFeatureSet());
    if (!featureSet) {
        vlf::log::error("Failed to create face feature set instance.");
        return -1;
    }

    // Create warper.
    fsdk::IDescriptorFactoryPtr descriptorFactory = fsdk::acquire(faceEngine->createDescriptorFactory());
    if (!descriptorFactory) {
        vlf::log::error("Failed to create face descriptor factory instance.");
        return -1;
    }
    fsdk::IWarperPtr warper = fsdk::acquire(descriptorFactory->createWarper(fsdk::DT_CNN));
    if (!warper) {
        vlf::log::error("Failed to create face warper instance.");
        return -1;
    }

    // Creating estimator
    fsdk::IEstimatorFactoryPtr estimatorFactory = fsdk::acquire(faceEngine->createEstimatorFactory());
    if (!estimatorFactory) {
        vlf::log::error("Failed to create face estimator factory instance.");
        return -1;
    }
    fsdk::IComplexEstimatorPtr complexEstimator =
            fsdk::acquire(static_cast<fsdk::IComplexEstimator*>(
                    estimatorFactory->createEstimator(fsdk::ET_COMPLEX)
            ));
    if (!complexEstimator) {
        vlf::log::error("Failed to create face complex estimator instance.");
        return -1;
    }
    fsdk::IQualityEstimatorPtr qualityEstimator =
            fsdk::acquire(static_cast<fsdk::IQualityEstimator*>(
                    estimatorFactory->createEstimator(fsdk::ET_QUALITY)
            ));
    if (!qualityEstimator) {
        vlf::log::error("Failed to create face quality estimator instance.");
        return -1;
    }

    // Load image.
    fsdk::Image image;
    if (!image.loadFromPPM(imagePath)) {
        vlf::log::error("Failed to load image: %s", imagePath);
        return -1;
    }

    // Need only R-channel image for feature extraction.
    fsdk::Image imageR;
    image.convert(imageR, fsdk::Format::R8);

    // We assume no more than 10 faces per image.
    enum { MaxDetections = 10 };

	fsdk::Detection detections[MaxDetections];
	int detectionsCount(MaxDetections);

    // Detect faces on the photo.
    fsdk::Result<fsdk::FSDKError> detectorResult =
            detector->detect(
                    image,
                    image.getRect(),
                    &detections[0],
                    &detectionsCount
            );
    if (detectorResult.isError()) {
        vlf::log::error("Failed tor create face detection. Reason: %s.", detectorResult.what());
        return -1;
    }
    vlf::log::info("Detections found: %d", detectionsCount);

    // Loop through all the faces.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
	    fsdk::Detection &detection = detections[detectionIndex];

	    std::cout << "Detection " << detectionIndex + 1
	            << "\nRect: x=" << detection.rect.x << " y=" << detection.rect.y
                <<" w=" << detection.rect.width << " h=" << detection.rect.height << std::endl;

        // Create feature set.
        fsdk::Result<fsdk::FSDKError> featureDetectorResult =
                featureDetector->detect(imageR, detection, featureSet);
        if (featureDetectorResult.isError()) {
            vlf::log::error("Failed to create feature set. Reason: %s.", featureDetectorResult.what());
            return -1;
        }

        // Get warped face from detection.
        fsdk::Image warp;
        fsdk::Result<fsdk::FSDKError> warperResult = warper->warp(image, detection, featureSet, warp);
        if (warperResult.isError()) {
            vlf::log::error("Failed to create warp. Reason: %s.", warperResult.what());
            return -1;
        }
        warp.saveAsPPM(("warp_" + std::to_string(detectionIndex) + ".ppm").c_str());
        
        // Quality estimating.
        float qualityOut;
        fsdk::Result<fsdk::FSDKError> qualityEstimatorResult = qualityEstimator->estimate(warp, &qualityOut);
        if(qualityEstimatorResult.isError()) {
            vlf::log::error("Failed to create quality estimating. Reason: %s.", qualityEstimatorResult.what());
            return -1;
        }
        std::cout << "Quality estimated\nQuality: " << qualityOut << std::endl;

        // Complex estimating.
        fsdk::ComplexEstimation complexEstimationOut;
        fsdk::Result<fsdk::FSDKError> complexEstimatorResult =
                complexEstimator->estimate(warp, complexEstimationOut);
        if(complexEstimatorResult.isError()) {
            vlf::log::error("Failed to create complex estimator. Reason: %s.", complexEstimatorResult.what());
            return -1;
        }
        std::cout << "Complex attributes estimated\n"
                "Gender: " << complexEstimationOut.gender << " (1 - man, 0 - woman)\n"
                "Natural skin color: " << complexEstimationOut.naturSkinColor
                << " (1 - natural color of skin, 0 - not natural color of skin color)\n"
                "Over exposed: " << complexEstimationOut.overExposed
                << " (1 - image is overexposed, 0 - image isn't overexposed)\n"
                "Wear glasses: " << complexEstimationOut.wearGlasses
                << " (1 - person wears glasses, 0 - person doesn't wear glasses)\n"
                "Age: " << complexEstimationOut.age
                << " (in years)\n"
                << std::endl;
    }

    return 0;
}
