#include "cameradetection.h"

CameraDetection::CameraDetection()
{
    cv::Mat a = cv::Mat::zeros(1,6, CV_8UC1);
}


std::vector<std::string> CameraDetection::detectCam(){
    std::vector<std::string> camAvailable;
    for (int i=0; i<5; i++){
        //测试摄像头是否可用，可用则加入camAvailable
        if (this->testCam(i)){
            camAvailable.push_back(std::to_string(i));
        }
    }
    return camAvailable;
}


std::string CameraDetection::getActivateCam(){
    return this->activateCamIndex;
}

void CameraDetection::activateCam(const std::string index){
    this->activateCamIndex = index;
}

/*
    * 测试摄像头是否可用
    * @param i:摄像头索引
    * @return:可用返回true，否则返回false
*/
bool CameraDetection::testCam(int i){
    cv::VideoCapture testcap(i);
    bool ret = false;
    if (!testcap.isOpened()) {
        return false;
    }
    else{
        cv::Mat frame;
        testcap >> frame;
        if (!frame.empty()){
            ret = true;
        }
    }
    testcap.release();
    return ret;
}
