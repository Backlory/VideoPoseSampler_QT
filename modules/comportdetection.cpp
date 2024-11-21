/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-10-14 09:40:13
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-11-21 10:04:48
 * @Description: 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
#include "comportdetection.h"
#include <iostream>
#include <QDebug>

int COMPortDetection::getActivateCOM() const{
    // 删除this->COMIndex的前三位，将剩下内容转为数字
    return std::atoi(this->COMIndex.substr(3).c_str());
}

void COMPortDetection::activateCOM(std::string comName) {
    this->COMIndex = idx2ports[comName];
}

// store the available ports
std::vector<std::string> COMPortDetection::getAvailablePorts() {
    this->ports.clear();
    this->idx2ports.clear();
    //

    QVector<QSerialPortInfo> vec;

    //查找可用的串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        // 检测端口列表变更
        QString portName = info.portName() + " (" + info.description() + ")";
        this->ports.push_back(portName.toStdString());
        this->idx2ports[portName.toStdString()] = info.portName().toStdString();
        vec.append(info);
    }

    return this->ports;
}
