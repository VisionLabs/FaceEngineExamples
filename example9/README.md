# Example 9
## What it does
This example demonstrates how to capture videostream from rgb + depth camera, using OpenCV with OPENNI2 support, create LivenessEngine core object and ComplexLiveness tests, pass frames to IComplexLiveness instance in order to retrieve result of the test.

## Prerequisites
*As said in the introduction page, this repository doesn't provide SDK headers, libraries
and tools; you have to obtain them from VisionLabs.*

This example assumes that you have read the **FaceEngine Handbook** and **LivenessEngine Handbook** already
(or at least have it somewhere nearby for reference) and know some core concepts,
like memory management, object ownership and life-time control. This sample will not explain
these aspects in detail.

This example requires installed OpenCV library with imgproc, highgui and videoio modules and OPENNI2 support, for additional information refer to https://opencv.org/.

## Example walkthrough
This example consists of the following stages:

### Stage 0. Preparations
Camera capturing and tuning, SDK initialization, liveness test creation.

### Stage 1. Main cycle
This stage is implemented in ```while(process)``` cycle.
At this stage we grab frames from video capture, convert colored frame from BGR to RGB, wrap both into FaceEngine structures,
pass Images to liveness instance and output colored and depth pictures with text hints to user via "Color"/"Depth" windows and console.
```C++
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
```
As the result we get flag whether or not liveness detection was successful.
if result == true then liveness is successful, otherwise not.

## Installation
Before running this example, it is necessary to place OpenNI2 folder with drivers near the executable.

## How to run
./Example9

