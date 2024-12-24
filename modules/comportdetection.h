#ifndef COMPORTDETECTION_H
#define COMPORTDETECTION_H

#include "../Aurora/INIFileRW.h"
#include <string>
#include <vector>
#include <map>
#include <QString>
#include <QVector>
#include <QSerialPort>
#include <QSerialPortInfo>

class COMPortDetection {
public:
    COMPortDetection(){};
    ~COMPortDetection(){};

    std::vector<std::string> getAvailablePorts();
    int getActivateCOM() const;
    void activateCOM(std::string);

private:
    std::string COMIndex = "-1";
    std::vector<std::string> ports;
    std::map<std::string, std::string> idx2ports;
};

#endif // COMPORTDETECTION_H
