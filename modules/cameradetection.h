#ifndef CAMERADETECTION_H
#define CAMERADETECTION_H

#include<vector>
#include<string>

#include "opencv2/opencv.hpp"

class CameraDetection{
public:
    CameraDetection();
    ~CameraDetection(){};

    std::string getActivateCam();
    bool activateCam(const int);
private:
    int activateCamIndex;
    bool testCam(int);

};

#endif // CAMERADETECTION_H
