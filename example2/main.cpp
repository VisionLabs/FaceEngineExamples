#include <FaceEngine.h>

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

    std::clog << "imagePath: \"" << imagePath << "\"" << std::endl;

    // Create config FaceEngine root SDK object.
    fsdk::ISettingsProviderPtr config;
    config = fsdk::acquire(fsdk::createSettingsProvider("./data/faceengine.conf"));
    if (!config) {
        std::cerr << "Failed to load face engine config instance." << std::endl;
        return -1;
    }

    // Create FaceEngine root SDK object.
    fsdk::IFaceEnginePtr faceEngine = fsdk::acquire(fsdk::createFaceEngine());
    if (!faceEngine) {
        std::cerr << "Failed to create face engine instance." << std::endl;
        return -1;
    }
    faceEngine->setSettingsProvider(config);
    faceEngine->setDataDirectory("./data/");

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

    // Create attribute estimator.
    fsdk::IAttributeEstimatorPtr attributeEstimator = fsdk::acquire(faceEngine->createAttributeEstimator());
    if (!attributeEstimator) {
        std::cerr << "Failed to create attribute estimator instance." << std::endl;
        return -1;
    }

    // Create quality estimator.
    fsdk::IQualityEstimatorPtr qualityEstimator = fsdk::acquire(faceEngine->createQualityEstimator());
    if (!qualityEstimator) {
        std::cerr << "Failed to create quality estimator instance." << std::endl;
        return -1;
    }

    // Create eye estimator.
    fsdk::IEyeEstimatorPtr eyeEstimator = fsdk::acquire(faceEngine->createEyeEstimator());
    if (!eyeEstimator) {
        std::cerr << "Failed to create eye estimator instance." << std::endl;
        return -1;
    }

    // Create head pose estimator.
    fsdk::IHeadPoseEstimatorPtr headPoseEstimator = fsdk::acquire(faceEngine->createHeadPoseEstimator());
    if (!headPoseEstimator) {
        std::cerr << "Failed to create head pose estimator instance." << std::endl;
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
    fsdk::Landmarks68 landmarks68[MaxDetections];

    // Detect faces in the image.
    fsdk::ResultValue<fsdk::FSDKError, int> detectorResult = faceDetector->detect(
            image,
            image.getRect(),
            &detections[0],
            &landmarks5[0],
            &landmarks68[0],
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

    // Loop through all the faces.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
	    std::cout << "Detection " << detectionIndex + 1 <<
	            "\nRect: x=" << detections[detectionIndex].rect.x <<
	            " y=" << detections[detectionIndex].rect.y <<
                " w=" << detections[detectionIndex].rect.width <<
                " h=" << detections[detectionIndex].rect.height << std::endl;

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
        
        // Get attribute estimate.
        fsdk::AttributeEstimation attributeEstimation;
        fsdk::Result<fsdk::FSDKError> attributeEstimatorResult = attributeEstimator->estimate(warp, attributeEstimation);
        if(attributeEstimatorResult.isError()) {
            std::cerr << "Failed to create attribute estimation. Reason: " << attributeEstimatorResult.what() << std::endl;
            return -1;
        }
        std::cout << "Attribure estimate:" <<
            "\ngender: " << attributeEstimation.gender << " (1 - man, 0 - woman)"
            "\nwearGlasses: " << attributeEstimation.wearGlasses <<
            " (1 - person wears glasses, 0 - person doesn't wear glasses)" <<
            "\nage: " << attributeEstimation.age << " (in years)" << std::endl;

        // Get quality estimate.
        fsdk::Quality qualityEstimation;
        fsdk::Result<fsdk::FSDKError> qualityEstimationResult = qualityEstimator->estimate(warp, qualityEstimation);
        if(qualityEstimationResult.isError()) {
            std::cerr << "Failed to create quality estimation. Reason: " << qualityEstimationResult.what() << std::endl;
            return -1;
        }
        std::cout << "Quality estimate:" <<
            "\nlight: " << qualityEstimation.light <<
            "\ndark: " << qualityEstimation.dark <<
            "\ngray: " << qualityEstimation.gray <<
            "\nblur: " << qualityEstimation.blur <<
            "\nquality: " << qualityEstimation.getQuality() << std::endl;

        // Get eye estimate.
        fsdk::EyeEstimation eyeEstimation[2];
        fsdk::Result<fsdk::FSDKError> eyeEstimationResult = eyeEstimator->estimate(
            warp,
            landmarks5[detectionIndex],
            eyeEstimation[0],
            eyeEstimation[1]
        );
        if(eyeEstimationResult.isError()) {
            std::cerr << "Failed to create eye estimation. Reason: " << eyeEstimationResult.what() << std::endl;
            return -1;
        }
        std::cout << "Eye estimate:" <<
            "\nleft eye state: " << static_cast<int>(eyeEstimation[0].eyeState) << " (0 - close, 1 - open, 2 - noteye)" <<
            "\nright eye state: " << static_cast<int>(eyeEstimation[1].eyeState) << " (0 - close, 1 - open, 2 - noteye)" <<
            std::endl;
        std::cout << std::endl;

         // Get head pose estimate.
        fsdk::HeadPoseEstimation headPoseEstimation;
        fsdk::Result<fsdk::FSDKError> headPoseEstimationResult = headPoseEstimator->estimate(
            landmarks68[detectionIndex],
            headPoseEstimation
        );
        if(headPoseEstimationResult.isError()) {
            std::cerr << "Failed to create head pose estimation. Reason: " << headPoseEstimationResult.what() << std::endl;
            return -1;
        }
        std::cout << "Head pose estimate:" <<
            "\npitch angle estimation: " << headPoseEstimation.pitch <<
            "\nyaw angle estimation: " << headPoseEstimation.yaw <<
            "\nroll angle estimation: " << headPoseEstimation.yaw <<
            std::endl;
        std::cout << std::endl;
    }

    return 0;
}
