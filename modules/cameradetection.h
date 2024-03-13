#ifndef CAMERADETECTION_H
#define CAMERADETECTION_H

#include<vector>
#include<string>

#include "opencv2/opencv.hpp"
//using namespace cv;

class CameraDetection{
public:
    CameraDetection();
    ~CameraDetection();

    std::vector<std::string> detectCam();
    void activateCam();
private:
    cv::VideoCapture * cap;
};

#endif // CAMERADETECTION_H
