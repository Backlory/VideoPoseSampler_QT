#include "cameradetection.h"

CameraDetection::CameraDetection()
{
    cv::Mat a = cv::Mat::zeros(1,6, CV_8UC1);
}


std::vector<std::string> CameraDetection::detectCam(){
    std::vector<std::string> camAvailable;
    return camAvailable;
}
void CameraDetection::activateCam(){
    //
    return;
}
