#ifndef COMPORTDETECTION_H
#define COMPORTDETECTION_H

#include "../3rdparty/Aurora/INIFileRW.h"

class COMPortDetection {
public:
    COMPortDetection();
    ~COMPortDetection();

    void detectCOM();
    void updateINIParam(int COM); //点击确定后，更新ini文件，然后NDI Initialize(true)
private:
    int portCOM = -1;
};

#endif // COMPORTDETECTION_H
