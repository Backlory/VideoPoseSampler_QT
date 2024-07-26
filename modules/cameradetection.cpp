#include "cameradetection.h"

CameraDetection::CameraDetection()
{}


std::string CameraDetection::getActivateCam(){
    std::string ret = std::to_string(this->activateCamIndex);
    return ret;
}

bool CameraDetection::activateCam(const int camIndex){
    if (testCam(camIndex)){
        this->activateCamIndex = camIndex;
        return true;
    }
    return false;
}

/*
    * 测试摄像头是否可用
    * @param i:摄像头索引
    * @return:可用返回true，否则返回false
*/
bool CameraDetection::testCam(int i){
    // 如果i是0 1 2 3 4 5
    cv::VideoCapture testcap;
    testcap.open(i);
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
        ret = true;
    }
    testcap.release();
    return ret;
}

