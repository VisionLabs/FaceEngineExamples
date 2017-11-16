#include <iostream>
#include <lsdk/LivenessEngine.h>

//This example require OpenCV highgui and imgproc module with OpenNI2 support for video capture and frame processing;
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

int main(int argc, char* argv[]) {

    //Capture rgb and depth camera using OPENNI2
    cv::VideoCapture capture(CV_CAP_OPENNI2);

    if (!capture.isOpened()){
        std::cout << "Couldn't capture video from camera " <<std::endl;
        return -1;
    }

    //Turn on pixel remapping
    if (capture.get(CV_CAP_PROP_OPENNI_REGISTRATION) == 0) capture.set(CV_CAP_PROP_OPENNI_REGISTRATION, 1);

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

    //Create liveness according to chosen type
    auto depthLiveness = acquire(livenessEngine->createComplexLiveness(CLA_DEPTH));

    std::clog << "Look straight into the camera" << std::endl;

    bool process = true;
    bool start = true;
    bool success = false;

    cv::Mat capturedImage, depthMap;
    
    while(process){
        //Grab frames
        capture.grab();
        capture.retrieve(capturedImage, CV_CAP_OPENNI_BGR_IMAGE);
        capture.retrieve(depthMap, CV_CAP_OPENNI_DEPTH_MAP);

        if((!capturedImage.empty())&&(!depthMap.empty())){
            cv::Mat rgb;
            cv::cvtColor(capturedImage,rgb,CV_BGR2RGB);
            
            //Wrap images
            Image color(rgb.cols, rgb.rows, fsdk::Format::R8G8B8, rgb.data);
            Image depth(depthMap.cols, depthMap.rows, fsdk::Format::R16, depthMap.data);
            
            //Update liveness state
            ResultValue<LSDKError,bool> result = depthLiveness->update(color,depth);
            
            if(result.isOk()){
                success = result.getValue();
                process = false;
            }

           //Output frames
           cv::namedWindow("Color", CV_WINDOW_AUTOSIZE);
           cv::namedWindow("Depth", CV_WINDOW_AUTOSIZE);
           cv::imshow("Color", capturedImage);
           cv::imshow("Depth", depthMap);
           cv::waitKey(1);

        } else process = false;
    }

    std::clog << "Liveness " << (success ? "successfull" : "unsuccessfull") << std::endl;
    capture.release();
    cv::waitKey(0);
    return success;
}
