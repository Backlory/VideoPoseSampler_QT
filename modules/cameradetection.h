#ifndef CAMERADETECTION_H
#define CAMERADETECTION_H

#include<vector>
#include<string>

#include "opencv2/opencv.hpp"

class CameraDetection{
public:
    CameraDetection();
    ~CameraDetection(){};

    std::vector<std::string> detectCam();
    std::string getActivateCam();
    void activateCam(const std::string);
private:
    bool testCam(int);
    std::string activateCamIndex;

};

#endif // CAMERADETECTION_H
