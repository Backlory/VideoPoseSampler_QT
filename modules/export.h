#ifndef EXPORT_H
#define EXPORT_H

#define CHUNK_SIZE 204800

#include <QString>
#include <QDir>

#include <WinSock2.h>
#include <QDebug>
#include "../3rdparty/Aurora/APIStructures.h"
#include "opencv2/opencv.hpp"
#include "Sophus/so3.hpp"
#include "boost/thread.hpp"

class Export{
    using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

public:
    Export();
    ~Export();

    void createFolder(const QString) const;

    template <typename T>
    void exportData(const int &, const cv::Mat&,
                    const std::vector<int>&, const std::map<int, T>&,
                    QString, QString) const;


    // ======== Socket ============
    bool SocketInit(std::string adddress = "127.0.0.1", int port = 2345);
    bool SocketClose();
    template <typename T>
    int exportSocketData(const int &, const cv::Mat &,
                          const std::vector<int> &, const std::map<int, T> &,
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
void Export::exportData(const int &frameIndex, const cv::Mat &frame,
                        const std::vector<int> &ndiHandle, const std::map<int, T> &ndiData, \
                        QString timeStamp, QString savePath) const {
    //assert(ndiHandle.size() == 4);
    //exportThread<T>(frameIndex, frame, ndiHandle, ndiData, timeStamp, savePath);
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
int Export::exportSocketData(const int &frameIndex, const cv::Mat &frame,
                      const std::vector<int> &ndiHandle, const std::map<int, T> &ndiData, \
                      QString timeStamp) const {
    // 检查是否连接中
    if (this->sockClient == INVALID_SOCKET) {
        return false;
    }

    // ===================================
    // 文本数据
    std::string msg_part1;
    QString temp;
    msg_part1 += "{\n";
    msg_part1 += "\"TimeStamp\": \"" + timeStamp.toStdString() + "\",\n";
    msg_part1 += "\"Poses\" : \n";
        msg_part1 += "\t{\n";
        if (ndiHandle.empty()) {
            msg_part1 += "\t\"0\": [-1],\n";
            msg_part1 += "\t\"1\": [-1],\n";
            msg_part1 += "\t\"2\": [-1],\n";
            msg_part1 += "\t\"3\": [-1]\n";
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
                msg_part1 += "\t\"" + std::to_string(i) +"\":" + temp.toStdString();
                if (i == 3)
                        msg_part1 += "\n";
                else
                        msg_part1 += ",\n";
            }
        }
        msg_part1 += "\t}\n";
    msg_part1 += "} \n";
    //图像数据
    std::vector<uchar> data_encode;
    std::string msg_part2;
    if(!frame.empty()) {
        cv::imencode(".jpg", frame, data_encode);
        msg_part2 = std::string(data_encode.begin(), data_encode.end());
    }
    else {
        msg_part2 = "abcdef";
    }
    
    // ===================================
    // 数据打包
    std::string sizePart1 = std::to_string(msg_part1.size());
    int size = sizePart1.size();
    for (int i = 0; i < 16 - size; i++) {
            sizePart1 = "0" + sizePart1;
    }
    std::string sizePart2 = std::to_string(msg_part2.size());
    int size2 = sizePart2.size();
    for (int i = 0; i < 16 - size2; i++) {
            sizePart2 = "0" + sizePart2;
    }

    //第一次接收：get
    char recvBuf[10] = {}; //接收
    if (recv(this->sockClient, recvBuf, 10, 0) <= 0) {
        qDebug() << "接收失败";
        return 1;  // 接收失败
    }
    std::string recvMsg(recvBuf); //转换
    if (recvMsg.compare("get") != 0) {
        qDebug() << "接收失败，所发送消息不是get";
        return 2;  // 接收失败，不是get
    }

    //第一次发送：size信号+msg_part1, 长度需<256
    std::string msg_Sizep1_Sizep2_P1 = sizePart1 + sizePart2 + msg_part1;
    int size_ = msg_Sizep1_Sizep2_P1.size();
    if (send(this->sockClient, msg_Sizep1_Sizep2_P1.c_str(), msg_Sizep1_Sizep2_P1.size(), 0) == -1) {
        qDebug() << "第一次发送失败";
        return 3;  // 发送失败
    }

    // 第二次发送
    size_t size2_ = msg_part2.size();
    size_t numChunks = (size2_ + CHUNK_SIZE - 1) / CHUNK_SIZE; //分包
    for (size_t i = 0; i < numChunks - 1; ++i){
        size_t start = i * CHUNK_SIZE;
        if (send(this->sockClient, msg_part2.data() + start, CHUNK_SIZE, 0) == -1) {
            qDebug() << "第二次发送失败(chunk错误)";
            return 4;  // 发送包失败
        }
    }
    // 将最后一个包补全到chunk_size
    size_t start = (numChunks - 1) * CHUNK_SIZE;
    for (size_t i = 0; i < CHUNK_SIZE - (size2_ - start); ++i){
        msg_part2 += "0";
    }
    if (send(this->sockClient, msg_part2.data() + start, CHUNK_SIZE, 0) == -1) {
        qDebug() << "第二次发送失败(chunk错误_end)";
        return 4;  // 发送包失败
    }

    return -1;
}


#endif // EXPORT_H
