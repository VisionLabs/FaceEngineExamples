#include <iostream>
#include <vlf/Types.h>
#include <FaceEngine.h>
#include <IEstimator.h>

int main(int argn, char** argv)
{
    if(argn != 2)
    {
        std::cout << "USAGE: " << argv[0] << " <imagePath>\n";
        return -1;
    }

    const std::string dataPath = "/opt/visionlabs/data/";

    vlf::Image image;
    if( !image.loadFromPPM(argv[1]) )
    {
        std::cout << "Cant load image: " << argv[1] << '\n';
        return -1;
    }

    // Need only R-channel image for feature extraction
    vlf::Image imageR;
    image.convert(imageR, vlf::Format::R8);

    // Create FaceEngine main object
    fsdk::Ref<fsdk::IFaceEngine> faceEngine = fsdk::acquire(fsdk::createFaceEngine());

    // Creating detector
    fsdk::Ref<fsdk::IDetectorFactory> detectorFactory = fsdk::acquire(faceEngine->createDetectorFactory());
    detectorFactory->setDataDirectory(dataPath.c_str());
    fsdk::Ref<fsdk::IDetector> detector = fsdk::acquire(detectorFactory->createDetector(fsdk::ODT_DPM));

    // Creating feature extractor
    fsdk::Ref<fsdk::IFeatureFactory> featureFactory = fsdk::acquire(faceEngine->createFeatureFactory());
    featureFactory->setDataDirectory(dataPath.c_str());
    fsdk::Ref<fsdk::IFeatureDetector> featureDetector = fsdk::acquire(featureFactory->createDetector(fsdk::FT_VGG));
    fsdk::Ref<fsdk::IFeatureSet> featureSet = fsdk::acquire(featureFactory->createFeatureSet());

    // Create warper
    fsdk::Ref<fsdk::IDescriptorFactory> descriptorFactory = fsdk::acquire(faceEngine->createDescriptorFactory());
    fsdk::Ref<fsdk::ICNNDescriptor> descriptor =
        fsdk::acquire(static_cast<fsdk::ICNNDescriptor*>(descriptorFactory->createDescriptor(fsdk::DT_CNN)));
    fsdk::Ref<fsdk::ICNNWarper> warper = 
        fsdk::acquire(static_cast<fsdk::ICNNWarper*>(descriptorFactory->createWarper(fsdk::DT_CNN)));

    // Creating estimator
    fsdk::Ref<fsdk::IEstimatorFactory> estimatorFactory = fsdk::acquire(faceEngine->createEstimatorFactory());
    estimatorFactory->setDataDirectory(dataPath.c_str());
    fsdk::Ref<fsdk::IComplexEstimator> complexEstimator =
        fsdk::acquire(static_cast<fsdk::IComplexEstimator*>(estimatorFactory->createEstimator(fsdk::ET_COMPLEX)));
    fsdk::Ref<fsdk::IQualityEstimator> qualityEstimator =
        fsdk::acquire(static_cast<fsdk::IQualityEstimator*>(estimatorFactory->createEstimator(fsdk::ET_QUALITY)));

    // Detecting faces on the photo
    fsdk::Detection detections[10];
    int count = 10;
    fsdk::Result<fsdk::FSDKError> res = detector->detect(image, image.getRect(), detections, &count);
    if(res.isError())
    {
        std::cout << "Face detection error\n";
        return -1;
    }
    std::cout << "Detections found: " << count << "\n\n";
    for(int i = 0; i < count; i++)
    {
	    fsdk::Detection& detection = detections[i];

	    std::cout << "Detection " << i << "\n";
	    std::cout << "Rect: x=" << detection.rect.x << " y=" << detection.rect.y
            << " w=" << detection.rect.width << " h=" << detection.rect.height << '\n';

        // Extracting face features
        fsdk::Result<fsdk::FSDKError> res = featureDetector->detect(imageR, detection, featureSet);
        if(res.isError())
        {
            std::cout << "Feature extraction error\n";
            continue;
        }

        // Get warped face from detection
        fsdk::Image warp;
        warper->warp(image, detection, featureSet, warp);

        warp.saveAsPPM(("warp_" + std::to_string(i) + ".ppm").c_str());
        
        // Quality estimating
        float qualityOut;
        res = qualityEstimator->estimate(warp, &qualityOut);
        if(res.isError())
        {
            std::cout << "Quality estimating error\n";
            return -1;
        }
        std::cout << "Quality estimated\n";
        std::cout << "Quality: " << qualityOut << "\n";

        // Complex attributes estimating
        fsdk::ComplexEstimation complexEstimationOut;
        res = complexEstimator->estimate(warp, complexEstimationOut);

        if(res.isError())
        {
            std::cout << "Complex attributes estimating error\n";
            return -1;
        }
        std::cout << "Complex attributes estimated\n";
        std::cout << "Gender: " << complexEstimationOut.gender << " (1 - man, 0 - woman)\n";
        std::cout << "Natural skin color: " << complexEstimationOut.naturSkinColor << " (1 - natural color of skin, 0 - not natural color of skin color)\n";
        std::cout << "Over exposed: " << complexEstimationOut.overExposed << " (1 - image is overexposed, 0 - image isn't overexposed)\n";
        std::cout << "Wear glasses: " << complexEstimationOut.wearGlasses << " (1 - person wears glasses, 0 - person doesn't wear glasses)\n";
        std::cout << "Age: " << complexEstimationOut.age << " (in years)\n";
        std::cout << '\n';
    }

    return 0;
}
