#include <stdio.h>     
#include <stdlib.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <lsdk/LivenessEngine.h>

//This example require OpenCV highgui and imgproc module for video capture and frame processing;
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//Liveness test is accomplished when all subtests are passed.

int main(int argc, char* argv[]) {

	// Parse command line arguments.
    // Arguments:
    // 1) webcamera number;
    // 2) number of liveness test
	
	 if (argc != 3) {
        std::cout << "Usage: "<<  argv[0] << " <camera number> <testNumber> \n"
                " *camera number - number of webcamera\n"
                " *testNumber - number of liveness tests in a row to preform\n"
                << std::endl;
        return -1;
    }
	
    int camNumber = atoi(argv[1]);
    float testNumber = (float)atof(argv[2]);

	cv::VideoCapture cap;
	cap.open(camNumber);

	if (!cap.isOpened()){
		std::cout << "Couldn't capture video from camera "<< camNumber <<std::endl;
		return -1;
	}

	using namespace fsdk;
	using namespace lsdk;

	// Factory objects.
	// These set up various SDK components.
	// Note: SDK defines smart pointers for various object types named like IInterfacePtr.
	// Such objects implement reference counting to manage their life time. The smart pointers
	// will ensure that reference counting functions are called appropriately and the objects
	// are properly destroyed after use.
	IFaceEnginePtr faceEngine = acquire(createFaceEngine("./data", "./data/faceengine.conf"));
	ILivenessEnginePtr livenessEngine = acquire(createLivenessEngine(faceEngine,("./data")));

	srand( time( 0 ) );

	//Vector of supported algorithms
	std::vector<LivenessAlgoritmType> all{
		LivenessAlgoritmType::LA_PITCH_DOWN,
		LivenessAlgoritmType::LA_PITCH_UP,
		LivenessAlgoritmType::LA_YAW_LEFT,
		LivenessAlgoritmType::LA_YAW_RIGHT,
		LivenessAlgoritmType::LA_MOUTH,
		LivenessAlgoritmType::LA_EYEBROW,
		LivenessAlgoritmType::LA_EYE
	};

	//Vector of corresponding actions
	std::vector<const char*> actions {
			"Nod your head",
			"Raise your head",
			"Turn left",
			"Turn right",
			"Open mouth",
			"Raise eyebrows",
			"Blink",
	};

	std::vector<LivenessAlgoritmType> chosen;
	std::vector<const char*> advices;

	//Range check
	if(testNumber>all.size()){
		std::cout << "Test number "<< testNumber <<" above maximum" <<std::endl;
		return -1;
	}
	if(testNumber<1){
		std::cout << "Test number "<< testNumber <<" below minimum"<<std::endl;
		return -1;
	}

	//Get testNumber randomly placed algorithms
	for(int i=0; i < testNumber; ++i){
		int num = rand() % all.size();

		chosen.push_back(all[num]);
		advices.push_back(actions[num]);

		all.erase(all.begin()+num);
		actions.erase(actions.begin()+num);
	}

	//Result flag
	bool success = true;

	//Preallocate storage frame for capture
	int height =static_cast<int>(cap.get(CV_CAP_PROP_FRAME_HEIGHT)+0.5);
	int width = static_cast<int>(cap.get(CV_CAP_PROP_FRAME_WIDTH)+0.5);
	
	cap.set(CV_CAP_PROP_FPS, 15.0);
	cv::Mat output(height, width, CV_8UC3);

	for (unsigned int i =0; i< chosen.size(); i++) {

		//Create liveness according to chosen type
		auto liveness = acquire(livenessEngine->createLiveness(chosen[i]));

		//Initial action
		std::clog << "Look straight into the camera" << std::endl;

		bool process = true;
		bool start = true;

		while(process){

			//Grab frame
			cap >> output;

			if(!output.empty()){

				cv::Mat rgb;
				cv::cvtColor(output,rgb,CV_BGR2RGB);

				//Wrap frame into fsdk container
				Image img(rgb.cols, rgb.rows, fsdk::Format::R8G8B8, rgb.data);

				ResultValue<LSDKError,bool> result = liveness->update(img);

				if((result.getResult()==lsdk::ERR_NOT_READY)&&start){

					start = false;
					//Ask for an action
					std::clog << advices[i] << std::endl;
				}

				if(result.isOk()){

					//Save result
					success = success & result.getValue();
					process = false;
				}

				//Output video
				cv::Mat mirror;
				cv::flip(output, mirror, 1);
				cv::namedWindow("Video", CV_WINDOW_AUTOSIZE);
				cv::imshow("Video", mirror);
				cv::waitKey(1);

			} else process = false;
		}
	}

	std::clog << "Liveness " << (success ? "successfull" : "unsuccessfull") << std::endl;
	cap.release();
	cv::waitKey(0);

	return success;
}