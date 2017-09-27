#include <FaceEngine.h>
#include <vlf/Log.h>

#include <iostream>

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

    vlf::log::info("imagePath: \"%s\".", imagePath);

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
    
    // Create DPM detector.
    fsdk::IDetectorPtr detector = fsdk::acquire(detectorFactory->createDetector(fsdk::ODT_DPM));
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
    
    // Create VGG feature detector.
    fsdk::IFeatureDetectorPtr featureDetector = fsdk::acquire(featureFactory->createDetector(fsdk::FET_VGG));
    if (!featureDetector) {
        vlf::log::error("Failed to create face featrure detector instance.");
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

    // Create estimator factory.
    fsdk::IEstimatorFactoryPtr estimatorFactory = fsdk::acquire(faceEngine->createEstimatorFactory());
    if (!estimatorFactory) {
        vlf::log::error("Failed to create face estimator factory instance.");
        return -1;
    }

    // Create quality estimator.
    fsdk::IQualityEstimatorPtr qualityEstimator =
            fsdk::acquire(static_cast<fsdk::IQualityEstimator*>(
                    estimatorFactory->createEstimator(fsdk::ET_QUALITY)
            ));
    if (!qualityEstimator) {
        vlf::log::error("Failed to create face quality estimator instance.");
        return -1;
    }
    
    // Create complex estimator.
    fsdk::IComplexEstimatorPtr complexEstimator =
            fsdk::acquire(static_cast<fsdk::IComplexEstimator*>(
                    estimatorFactory->createEstimator(fsdk::ET_COMPLEX)
            ));
    if (!complexEstimator) {
        vlf::log::error("Failed to create face complex estimator instance.");
        return -1;
    }

    // Load image.
    fsdk::Image image;
    if (!image.loadFromPPM(imagePath)) {
        vlf::log::error("Failed to load image: \"%s\".", imagePath);
        return -1;
    }

    // Create grayscale image.
    fsdk::Image imageR;
    image.convert(imageR, fsdk::Format::R8);
    if (!imageR) {
        vlf::log::error("Conversion to grayscale has failed.");
        return -1;
    }

    vlf::log::info("Detecting faces.");

    // Detect no more than 10 faces in the image.
    enum { MaxDetections = 10 };
	fsdk::Detection detections[MaxDetections];
	int detectionsCount(MaxDetections);

    // Detect faces in the image.
    fsdk::ResultValue<fsdk::FSDKError, int> detectorResult =
            detector->detect(
                    imageR,
                    imageR.getRect(),
                    &detections[0],
                    detectionsCount
            );
    if (detectorResult.isError()) {
        vlf::log::error("Failed to detect face detection. Reason: %s.", detectorResult.what());
        return -1;
    }
    detectionsCount = detectorResult.getValue();
    vlf::log::info("Found %d face(s).", detectionsCount);

    // Create feature set.
    fsdk::IFeatureSetPtr featureSet = fsdk::acquire(featureFactory->createFeatureSet());
    if (!featureSet) {
        vlf::log::error("Failed to create face feature set instance.");
        return -1;
    }

    // Loop through all the faces.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
	    fsdk::Detection &detection = detections[detectionIndex];

	    std::cout << "Detection " << detectionIndex + 1
	            << "\nRect: x=" << detection.rect.x << " y=" << detection.rect.y
                <<" w=" << detection.rect.width << " h=" << detection.rect.height << std::endl;

        // Detect feature set.
        fsdk::Result<fsdk::FSDKError> featureDetectorResult =
                featureDetector->detect(imageR, detection, featureSet);
        if (featureDetectorResult.isError()) {
            vlf::log::error("Failed to detect feature set. Reason: %s.", featureDetectorResult.what());
            return -1;
        }

        // Estimate confidence score of feature set.
        if (featureSet->getConfidence() < confidenceThreshold) {
            vlf::log::info("Face detection succeeded, but confidence score of feature set is small.");
            continue;
        }

        // Get warped face from detection.
        fsdk::Image warp;
        fsdk::Result<fsdk::FSDKError> warperResult = warper->warp(image, detection, featureSet, warp);
        if (warperResult.isError()) {
            vlf::log::error("Failed to create warped face. Reason: %s.", warperResult.what());
            return -1;
        }
        
        // Save warped face.
        warp.saveAsPPM(("warp_" + std::to_string(detectionIndex) + ".ppm").c_str());
        
        // Get quality estimate.
        float qualityOut;
        fsdk::Result<fsdk::FSDKError> qualityEstimatorResult = qualityEstimator->estimate(warp, &qualityOut);
        if(qualityEstimatorResult.isError()) {
            vlf::log::error("Failed to get quality estimate. Reason: %s.", qualityEstimatorResult.what());
            return -1;
        }
        std::cout << "Quality estimated\nQuality: " << qualityOut << std::endl;

        // Get complex estimate.
        fsdk::ComplexEstimation complexEstimationOut;
        fsdk::Result<fsdk::FSDKError> complexEstimatorResult =
                complexEstimator->estimate(warp, complexEstimationOut);
        if(complexEstimatorResult.isError()) {
            vlf::log::error("Failed to get complex estimate. Reason: %s.", complexEstimatorResult.what());
            return -1;
        }
        std::cout << "Complex attributes estimated\n"
                "Gender: " << complexEstimationOut.gender << " (1 - man, 0 - woman)\n"
                "Wear glasses: " << complexEstimationOut.wearGlasses
                << " (1 - person wears glasses, 0 - person doesn't wear glasses)\n"
                "Age: " << complexEstimationOut.age
                << " (in years)\n"
                << std::endl;
    }

    return 0;
}
