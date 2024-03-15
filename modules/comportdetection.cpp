#include "comportdetection.h"


int COMPortDetection::getActivateCOM() const{
    return this->COMIndex;
}

void COMPortDetection::activateCOM(std::string comName) {
    this->COMIndex = idx2ports[comName];
}

// store the available ports
std::vector<std::string> COMPortDetection::getAvailablePorts() {
    this->ports.clear();
    this->idx2ports.clear();
    //
    boost::asio::io_context io;
    boost::asio::serial_port serial(io);
    for (unsigned int i = 0; i < 16; ++i) {
        std::string portName = "COM" + std::to_string(i);
        try {
            serial.open(portName);
            if (serial.is_open()) {
                this->ports.push_back(portName);
                this->idx2ports[portName] = i;
                serial.close();
            }
        }
        catch (const boost::system::system_error& e) {
            // Failed to open the port, so it doesn't exist
        }
    }
    return this->ports;
}
