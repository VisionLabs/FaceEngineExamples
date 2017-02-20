#include <iostream>
#include <vlf/Types.h>
#include <FaceEngine.h>
#include <IEstimator.h>

using namespace vlf;

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

	// Create FaceEngine main object
	fsdk::Ref<fsdk::IFaceEngine> faceEngine = acquire(fsdk::createFaceEngine());

	// Creating estimator
	fsdk::Ref<fsdk::IEstimatorFactory> estimatorFactory = acquire(faceEngine->createEstimatorFactory());
	estimatorFactory->setDataDirectory(dataPath.c_str());
	fsdk::Ref<fsdk::IQualityEstimator> qualityEstimator =
		acquire(static_cast<fsdk::IQualityEstimator*>(estimatorFactory->createEstimator(fsdk::ET_QUALITY)));

	// Quality estimating
	float qualityOut;
	Result<FSDKError> res = qualityEstimator->estimate(image, &qualityOut);

	if(res.isError())
	{
		std::cout << "Complex attributes estimating error\n";
		return -1;
	}
	std::cout << "Quality estimated\n";
	std::cout << "Quality: " << qualityOut << "\n";

	return 0;
}
