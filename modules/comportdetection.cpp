#include "comportdetection.h"
#include <iostream>
#include <QDebug>

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
    boost::system::error_code ec;
    boost::asio::io_service m_IoServie;
    for (unsigned int i = 0; i < 99; ++i) {
        std::string portName = "COM" + std::to_string(i);
        qDebug() << portName;
        //
        try{
            boost::asio::serial_port serialPort(m_IoServie);
            serialPort.open(portName);
            if (serialPort.is_open()) {
                this->ports.push_back(portName);
                this->idx2ports[portName] = i;
                serialPort.close();
            }

        } catch (std::exception& e){
            ;
        }
    }

    /*
    boost::asio::io_context io;
    boost::asio::serial_port serial(io);
    for (unsigned int i = 0; i < 99; ++i) {
        std::string portName = "COM" + std::to_string(i);
        qDebug() << portName;
        try {
            serial.open(portName);
            if (serial.is_open()) {
                this->ports.push_back(portName);
                this->idx2ports[portName] = i;
                serial.close();
            }
        }
        catch (const boost::system::system_error& e) {
            qDebug() << e.what();
        }
    }
*/
    return this->ports;
}
