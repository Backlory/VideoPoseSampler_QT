/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-12-02 14:12:06
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-12-02 19:12:26
 * @Description: 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
#include "localdwpplayer.h"

std::string numberfill6(int num) {
    if (num < 10) {
        return "00000" + std::to_string(num);
    }
    else if (num < 100) {
        return "0000" + std::to_string(num);
    }
    else if (num < 1000) {
        return "000" + std::to_string(num);
    }
    else if (num < 10000) {
        return "00" + std::to_string(num);
    }
    else if (num < 100000) {
        return "0" + std::to_string(num);
    }
    else {
        return std::to_string(num);
    }
}

LocalDwPPlayer::LocalDwPPlayer() {
    _ifLoaded = false;
    _nowIdx = 0;
    _ifRecurrent = false;
    _waitMS = 1000.0 / 30;
    _ifPlaying = false;
}

LocalDwPPlayer::~LocalDwPPlayer() {
    unloadDwP();
}

bool LocalDwPPlayer::loadDwP(const std::string &dwpPath) {
    if (_ifDwPLegal(dwpPath)) {
        this->dwpPath = dwpPath;
        _ifLoaded = true;
        _nowIdx = 0;
        return true;
    }
    return false;
}

bool LocalDwPPlayer::unloadDwP() {
    if (_ifLoaded) {
        _ifLoaded = false;
        _nowIdx = 0;
        dwpPath.clear();
        return true;
    }
    return false;
}

void LocalDwPPlayer::setRecurrence(bool ifRecurrent) {
    _ifRecurrent = ifRecurrent;
}

void LocalDwPPlayer::setFPS(float fps) {
    if (fps <= 0) {
        _waitMS = 1000.0 / 30;
    }
    else {
        _waitMS = 1000.0 / fps;
    }
}

void LocalDwPPlayer::jumpTo(int idx) {
    if (_ifLoaded) {
        if (idx >= 0 && idx < getLength()) {
            _nowIdx = idx;
        }
    }
}

bool LocalDwPPlayer::isReadyToGrab() {
    if (_ifLoaded) {
        return true;
    }
    return false;
}

void LocalDwPPlayer::flipFlag() {
    _ifPlaying = !_ifPlaying;
}

int LocalDwPPlayer::getLength() {
    if (_ifLoaded) {
        return this->_length;
    }
    return 0;
}

int LocalDwPPlayer::getNowIdx() {
    return _nowIdx;
}

bool LocalDwPPlayer::isPlaying(){
    return _ifPlaying;
}

bool LocalDwPPlayer::getOneFrame(cv::Mat &frame,
                                 std::vector<int> &ndiHandle,
                                 std::map<int, data_ptr7> &poseMap,
                                 std::map<int, double> &poseMapErr) {
    if (_ifLoaded) {
        // 获取时间
        auto start = std::chrono::steady_clock::now();
        try {
            // TODO: get frame from dwp file
            std::string frameName = this->dwpPath + "\\frame\\" + numberfill6(_nowIdx) + ".jpg";
            frame = cv::imread(frameName);
            
            ndiHandle.clear();
            poseMap.clear();
            // 加载到qt的json，解析
            std::string poseName = this->dwpPath + "\\PoseWithTimeStamp\\" + numberfill6(_nowIdx) + ".json";
            QFile file(QString::fromStdString(poseName));
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "Open file failed!";
                throw "Open file failed!";
            }
            QTextStream stream(&file);
            QString jsonString = stream.readAll();
            file.close();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
            QJsonObject jsonObject = jsonDoc.object();
            QJsonObject posesObject = jsonObject.value("Poses").toObject();

            // 读取"Poses"下的"0/1/2/3"键对应的7个值(x,y,z,q0,qx,qy,qz)，并保存到poseMap的0号位置
            for (int i = 0; i < 4; i++) {
                QJsonArray poseArray = posesObject.value(QString::number(i)).toArray();
                if (poseArray.size() == 7) {
                    QuatTransformationStruct pose;
                    pose.translation.x = poseArray.at(0).toDouble();
                    pose.translation.y = poseArray.at(1).toDouble();
                    pose.translation.z = poseArray.at(2).toDouble();
                    pose.rotation.q0 = poseArray.at(3).toDouble();
                    pose.rotation.qx = poseArray.at(4).toDouble();
                    pose.rotation.qy = poseArray.at(5).toDouble();
                    pose.rotation.qz = poseArray.at(6).toDouble();
                    // 将pose转换为shared_ptr并存储到poseMap中
                    data_ptr7 posePtr = std::make_shared<QuatTransformationStruct>(pose);
                    poseMap.insert(std::pair<int, data_ptr7>(i+10, posePtr));
                    poseMapErr.insert(std::pair<int, double>(i+10, 0));
                    ndiHandle.push_back(i+10);
                }
                else {
                    ndiHandle.push_back(-1);
                }
            }
        }
        catch (const char* msg) {
            std::cerr << msg << std::endl;
            return false;
        }

        // 推进
        if (_ifPlaying){
            _nowIdx++;
        }
        if (_nowIdx >= getLength()-1) {
            if (_ifRecurrent) {
                _nowIdx = 0;
            }
            else {
                _nowIdx = getLength()-1;
            }
        }

        // 模拟等待
        while (false) {
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            if (duration.count() >= _waitMS) {
                break;
            }
            else {
                qDebug()<<duration.count();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        return true;
    }
    else {
        return false;
    }
}

int getFileNum(const std::string& path) {
    struct _finddata_t fileinfo;
    std::string searchPath = path + "\\*.*";
    intptr_t handle = _findfirst(searchPath.c_str(), &fileinfo);
    int fileNum = 0;

    if (handle == -1) {
        return 0;
    }
    do {
        if (!(fileinfo.attrib & _A_SUBDIR)) {
            fileNum++;
        }
    } while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);
    return fileNum;
}

bool LocalDwPPlayer::_ifDwPLegal(const std::string &dwpPath) {
    std::string pathFrame = dwpPath + "\\frame";
    std::string pathPose = dwpPath + "\\PoseWithTimeStamp";
    
    // 检测1：dwpPath下是否存在frame、PoseWithTimeStamp文件夹
    if (_access(pathFrame.c_str(), 0) == -1 || _access(pathPose.c_str(), 0) == -1) {
        return false;
    }
    else{
        // 检测2：frame文件夹下图片数目是否与PoseWithTimeStamp文件夹下pose数目一致
        int frameNum = getFileNum(pathFrame);
        int poseNum = getFileNum(pathPose);
        if (frameNum != poseNum) {
            return false;
        }
        else {
            this->_length = frameNum;
            // 检测3：pose文件夹下编号是否连续，只需要检测第一位和最后一位即可
            for (int i = 0; i <= frameNum; i=i+frameNum-1) {
                std::string posePath = pathPose + "\\";
                posePath += numberfill6(i) + ".json";
                if (_access(posePath.c_str(), 0) == -1) {
                    return false;
                }
            }
        }
    }
    return true;
}
