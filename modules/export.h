#ifndef EXPORT_H
#define EXPORT_H

#include <QString>
#include <QDir>

#include <WinSock2.h>

#include "../3rdparty/Aurora/APIStructures.h"
#include "opencv2/opencv.hpp"
#include "Sophus/so3.hpp"
#include "boost/thread.hpp"

class Export{

    using data_ptr4 = std::shared_ptr<Eigen::Matrix4d>;
    using data_ptr6 = std::shared_ptr<Sophus::Vector6d>;
    using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

public:
    Export();
    ~Export();

    void createFolder(const QString) const;

    static void d7to4(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr4> &d4);
    static void d7to6(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr6> &d6);

    template <typename T>
    void exportData(const int, const cv::Mat,
                    const std::vector<int>, const std::map<int, T>,
                    QString, QString) const;


    // ======== Socket ============
    bool SocketInit(std::string adddress = "127.0.0.1", int port = 2345);
    bool SocketClose();
    template <typename T>
    int exportSocketData(const int, const cv::Mat,
                          const std::vector<int>, const std::map<int, T>,
                          QString) const;

private:
    void saveFrame(const QString, const cv::Mat, const QString ) const;
    template <typename T>
    void exportThread(const int frameIndex, const cv::Mat frame,
                      const std::vector<int>, const std::map<int, T> ndiData,
                  const QString timeStamp, const QString savePath) const;

    template <typename T>
    void savePoseWithTimeStamp(const std::vector<int> ndiHandle, const std::map<int, T> ndiData,
                            const QString timeStamp, const QString filePath) const ;
    //
    // ======== Socket ============
    WSADATA wsaData;
    SOCKET sockServer;
    SOCKET sockClient;
    SOCKADDR_IN addrServer;
    //服务器
    
};


template <typename T>
void ndiData2QString(const T data, QString &);


template <typename T>
void Export::exportData(const int frameIndex, const cv::Mat frame,
                        const std::vector<int> ndiHandle, const std::map<int, T> ndiData, \
                        QString timeStamp, QString savePath) const {
    //assert(ndiHandle.size() == 4);
    exportThread<T>(frameIndex, frame, ndiHandle, ndiData, timeStamp, savePath);
    boost::thread td(&Export::exportThread<T>, this, frameIndex, frame, ndiHandle, ndiData, timeStamp, savePath);
}

template <typename T1>
void Export::exportThread(const int frameIndex, const cv::Mat frame,
                          const std::vector<int> ndiHandle, const std::map<int, T1> ndiData,
                          const QString timeStamp, const QString savePath) const {
    QString frameIndexStr = QString::number(frameIndex);
    while (frameIndexStr.length() < 6) {
        frameIndexStr = "0" + frameIndexStr;
    }
    createFolder(savePath);
    //
    if (!frame.empty())
        saveFrame(frameIndexStr, frame, savePath);
    //
    if (!ndiHandle.empty()){
        QString filePath = savePath + "/PoseWithTimeStamp/" + frameIndexStr + ".json";
        savePoseWithTimeStamp<T1>(ndiHandle, ndiData, timeStamp, filePath);
    }
}

template <typename T>
void Export::savePoseWithTimeStamp(const std::vector<int> ndiHandle, const std::map<int, T> ndiData, \
                                   const QString timeStamp, const QString filePath) const {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString temp;
        out << "{\n";
        out << "\"TimeStamp\": \"" << timeStamp << "\",\n";
        //
        out << "\"Poses\" : \n";
            out << "\t{\n";
            if (ndiHandle.empty()) {
                out << "\t\"0\": [-1],\n";
                out << "\t\"1\": [-1],\n";
                out << "\t\"2\": [-1],\n";
                out << "\t\"3\": [-1]\n";
            }
            else{
                for(int i=0; i<4;i++){
                    temp.clear();
                    if(ndiHandle[i]>0){
                        T data = ndiData.at(ndiHandle[i]);
                        ndiData2QString<T>(data, temp);
                    }
                    else{
                        temp = "[-1]";
                    }
                    out << "\t\"" + QString::number(i) +"\":" + temp;
                    if (i == 3)
                        out << "\n";
                    else
                        out << ",\n";
                }
            }
            out << "\t}\n";
        //
        out << "} \n";
        file.close();
    }
}

template <typename T>
int Export::exportSocketData(const int frameIndex, const cv::Mat frame,
                      const std::vector<int> ndiHandle, const std::map<int, T> ndiData, \
                      QString timeStamp) const {
    // 检查是否连接中
    if (this->sockClient == INVALID_SOCKET) {
        return false;
    }
    //图像压缩
    std::vector<uchar> data_encode;
    std::string binary_data;
    if(!frame.empty()) {
        cv::imencode(".jpg", frame, data_encode);
        binary_data = std::string(data_encode.begin(), data_encode.end());
    }
    else {
        binary_data = "abcdef";
    }

    // 数据打包
    std::string message;
    //
    QString temp;
    message += "{\n";
    message += "\"TimeStamp\": \"" + timeStamp.toStdString() + "\",\n";
    message += "\"Poses\" : \n";
        message += "\t{\n";
        if (ndiHandle.empty()) {
            message += "\t\"0\": [-1],\n";
            message += "\t\"1\": [-1],\n";
            message += "\t\"2\": [-1],\n";
            message += "\t\"3\": [-1]\n";
        }
        else{
            for(int i=0; i<4;i++){
                temp.clear();
                if(ndiHandle[i]>0){
                        T data = ndiData.at(ndiHandle[i]);
                        ndiData2QString<T>(data, temp);
                }
                else{
                        temp = "[-1]";
                }
                message += "\t\"" + std::to_string(i) +"\":" + temp.toStdString();
                if (i == 3)
                        message += "\n";
                else
                        message += ",\n";
            }
        }
        message += "\t}\n";
    message += "} \n";
    // 尺寸
    std::string sizeJson = std::to_string(message.size());
    int size = sizeJson.size();
    for (int i = 0; i < 16 - size; i++) {
            sizeJson = "0" + sizeJson;
    }
    std::string sizeFrame = std::to_string(binary_data.size());
    int size2 = sizeFrame.size();
    for (int i = 0; i < 16 - size2; i++) {
            sizeFrame = "0" + sizeFrame;
    }
    message = sizeJson + sizeFrame + message + binary_data;

    // 发送、确认收到
    if (message.size() < 200*1024){
        // 先准备接受
        char recvBuf[10] = {};
            if (recv(this->sockClient, recvBuf, 10, 0) == -1) {
            return 1;
        }
        std::string recvMsg(recvBuf);
        if (recvMsg.compare("get") != 0) {
            return 2;
        }

        // 发送
        if (send(this->sockClient, message.c_str(), message.size(), 0) == -1) {
            return 3;
        }
//        // 再接受
//        char recvBuf2[35] = {};
//        if (recv(this->sockClient, recvBuf2, 35, 0) == -1) {
//            return 4;
//        }
//        std::string expectedResponse = "OK" + sizeJson + sizeFrame;
//        std::string actualResponse(recvBuf2);
//        if (actualResponse.compare(expectedResponse) != 0) {
//            return 5;
//        }
        return -1;
    }
    return -1;
}

#endif // EXPORT_H
