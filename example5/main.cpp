#include <FaceEngine.h>
#include <vlf/Log.h>

#include <QImage>
#include <QPainter>
#include <QPen>
#include <iostream>

// Helper function to convert Qt image to FSDK image.
fsdk::Image convertImage(const QImage &sourceImage);

int main(int argc, char *argv[])
{
    // Facial feature detection confidence threshold.
    const float confidenceThreshold = 0.25f;

    // Parse command line arguments.
    // Arguments:
    // 1) path to a first image.
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

    // Create estimator factory.
    fsdk::IEstimatorFactoryPtr estimatorFactory = fsdk::acquire(faceEngine->createEstimatorFactory());
    if (!estimatorFactory) {
        vlf::log::error("Failed to create face estimator factory instance.");
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

    // Create quality estimator.
    fsdk::IQualityEstimatorPtr qualityEstimator =
            fsdk::acquire(static_cast<fsdk::IQualityEstimator*>(
                    estimatorFactory->createEstimator(fsdk::ET_QUALITY)
            ));
    if (!qualityEstimator) {
        vlf::log::error("Failed to create face quality estimator instance.");
        return -1;
    }

    // Load source image.
    QImage sourceImage;
    if (!sourceImage.load(imagePath)) {
        vlf::log::error("Failed to load image: \"%s\".", imagePath);
        return -1;
    }

    // Convert Qt image to FSDK image.
    fsdk::Image image = convertImage(sourceImage);

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
        vlf::log::error("Failed tor create face detection. Reason: %s.", detectorResult.what());
        return -1;
    }
    detectionsCount = detectorResult.getValue();
    vlf::log::info("Found %d face(s).", detectionsCount);
    
    // Create image with face detection and marked detection points.
    QPainter painter;
    painter.begin(&sourceImage);
    QPen penDetection, penPoint;
    penDetection.setWidth(3);
    penDetection.setBrush(Qt::green);
    penPoint.setWidth(5);
    penPoint.setBrush(Qt::blue);

    // Feature set.
    fsdk::IFeatureSetPtr featureSet(nullptr);

    // Loop through all the faces.
    for (int detectionIndex = 0; detectionIndex < detectionsCount; ++detectionIndex) {
	    fsdk::Detection &detection = detections[detectionIndex];
	    fsdk::IMTCNNDetector::Landmarks &landmark = landmarks[detectionIndex];

	    std::cout << "Detection " << detectionIndex + 1 << "\n"
	            << "Rect: x=" << detection.rect.x << " y=" << detection.rect.y
                << " w=" << detection.rect.width << " h=" << detection.rect.height << std::endl;

        // Estimate confidence score of face detection.
        if (detection.score < confidenceThreshold) {
            vlf::log::info("Face detection succeeded, but confidence score of detection is small.");
            continue;
        }

        // Create feature set.
        featureSet = fsdk::acquire(featureFactory->createFeatureSet(landmark, detection.score));
        if (!featureSet) {
            vlf::log::error("Failed to create face feature set instance.");
            return -1;
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
            vlf::log::error("Failed to create quality estimating. Reason: %s.", qualityEstimatorResult.what());
            return -1;
        }
        std::cout << "Quality estimated\nQuality: " << qualityOut << std::endl;

        // Get complex estimate.
        fsdk::ComplexEstimation complexEstimationOut;
        fsdk::Result<fsdk::FSDKError> complexEstimatorResult =
                complexEstimator->estimate(warp, complexEstimationOut);
        if(complexEstimatorResult.isError()) {
            vlf::log::error("Failed to create complex estimator. Reason: %s.", complexEstimatorResult.what());
            return -1;
        }
        std::cout << "Complex attributes estimated\n"
                "Gender: " << complexEstimationOut.gender << " (1 - man, 0 - woman)\n"
                "Wear glasses: " << complexEstimationOut.wearGlasses
                << " (1 - person wears glasses, 0 - person doesn't wear glasses)\n"
                "Age: " << complexEstimationOut.age
                << " (in years)\n"
                << std::endl;

        painter.setPen(penDetection);
        painter.drawRect(detection.rect.x, detection.rect.y, detection.rect.width, detection.rect.height);

        painter.setPen(penPoint);
        painter.drawPoint(
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkLeftEye].x + detection.rect.x,
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkLeftEye].y + detection.rect.y
        );
        painter.drawPoint(
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkRightEye].x + detection.rect.x,
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkRightEye].y + detection.rect.y
        );
        painter.drawPoint(
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkNose].x + detection.rect.x,
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkNose].y + detection.rect.y
        );
        painter.drawPoint(
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkMouthLeftCorner].x + detection.rect.x,
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkMouthLeftCorner].y + detection.rect.y
        );
        painter.drawPoint(
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkMouthRightCorner].x + detection.rect.x,
                landmark.landmarks[fsdk::IMTCNNDetector::Landmarks::LandmarkMouthRightCorner].y + detection.rect.y
        );
    }

    painter.end();
    sourceImage.save("face_detection.png");

    return 0;
}

fsdk::Image convertImage(const QImage &sourceImage) {
    fsdk::Image colorImage(
        sourceImage.width(),
        sourceImage.height(),
        fsdk::Format::R8G8B8
    );

    // Do not rely on Qt image format conversion.
    // This code isn't very fast but does the right
    // thing in all cases.
    uchar *data = colorImage.getDataAs<uchar>();
    for (int y = 0; y < colorImage.getHeight(); ++y) {
        for(int x = 0; x < colorImage.getWidth(); ++x) {
            union rgba {
                uchar ch[4];
                uint dword;
            };

            rgba pixel;
            pixel.dword = sourceImage.pixel(x, y);

            uchar *ptr = data + (y * colorImage.getWidth() + x) * 3;

            ptr[0] = pixel.ch[2]; // r-g-b order
            ptr[1] = pixel.ch[1];
            ptr[2] = pixel.ch[0];
        }
    }

    return colorImage;
}
