#ifndef COMPORTDETECTION_H
#define COMPORTDETECTION_H

#include "../3rdparty/Aurora/INIFileRW.h"
#include <string>
#include <vector>
#include <map>
#include "boost/asio.hpp"


class COMPortDetection {
public:
    COMPortDetection(){};
    ~COMPortDetection(){};

    std::vector<std::string> getAvailablePorts();
    int getActivateCOM() const;
    void activateCOM(std::string);

private:
    int COMIndex = -1;
    std::vector<std::string> ports;
    std::map<std::string, int> idx2ports;
};

#endif // COMPORTDETECTION_H
