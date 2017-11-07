# Example 8
## What it does
This example demonstrates how to capture videostream from web camera, using OpenCV, create LivenessEngine core object and Liveness tests, pass frames to ILiveness instances in order to retrieve result of the test.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries
and tools; you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** and **LivenessEngine Handbook** already
(or at least have it somewhere nearby for reference) and know some core concepts,
like memory management, object ownership and life-time control. This sample will not explain
these aspects in detail.

This example requires installed OpenCV library with imgproc, highgui and videoio modules, for additional information refer to https://opencv.org/.

## Example walkthrough
This example consists of the following stages:

### Stage 0. Preparations
Argument parsing, SDK initialization, testing sequence preparation, video capturing.

### Stage 1. Main cycle
This stage is implemented in ```while(process)``` cycle.
At this stage we grab frame from video capture, covert it from BGR to RGB, wrap it into FaceEngine structures,
pass Image to liveness instance and output picture and action calls to user via Video window and console.
```C++
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
```
As the result we get flag whether or not liveness detection was successful.
This cycle repeats N times (N is specified as program argument).
Program result equals to Bitwise AND of every cycle iteration result.
if result == true then liveness is successful, otherwise not.

## How to run
./Example8 <camera_number> <descriptor2.xpk> threshold

