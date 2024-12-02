#ifndef EXPORTOPENIGTLINK_H
#define EXPORTOPENIGTLINK_H

#include <QString>
#include <QDir>

#include <QDebug>
#include "../3rdparty/Aurora/APIStructures.h"
#include "opencv2/opencv.hpp"


#include "igtlOSUtil.h"
#include "igtlImageMessage.h"
#include "igtlStringMessage.h"
#include "igtlServerSocket.h"


class ExportOpenIGTLink{
    using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

public:
    ExportOpenIGTLink();
    ~ExportOpenIGTLink();
    bool Init(int port = 2345);
    bool Close();
    void getAddress(std::string &);
    template <typename T>
    int exportOpenIGTLinkData(const int &, const cv::Mat &,
                          const std::vector<int> &, const std::map<int, T> &,
                          QString) const;
private:
    igtl::ServerSocket::Pointer openIGLLinkServer;
    igtl::Socket::Pointer openIGLLinkSocket;
};


template <typename T>
void ndiData2QStringOpenIGTLink(const T data, QString &);

/*监听进程*/
template <typename T>
int ExportOpenIGTLink::exportOpenIGTLinkData(const int &frameIndex, const cv::Mat &frame,
                      const std::vector<int> &ndiHandle, const std::map<int, T> &ndiData, \
                      QString timeStamp) const {
    // 检查是否连接中
    if (this->openIGLLinkSocket.IsNull()) {
        return 8;
    }
    // 获取新时间戳
    auto currentTimestamp = QDateTime::currentMSecsSinceEpoch();
    unsigned int sec = static_cast<unsigned int>(currentTimestamp / 1000);
    unsigned int frac = static_cast<unsigned int>(currentTimestamp % 1000);


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
                    ndiData2QStringOpenIGTLink<T>(data, temp);
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


    // ===================================
    // 将opencv的cv::Mat转换为igtl的ImageMessage
    igtl::ImageMessage::Pointer imgMsg = igtl::ImageMessage::New();
    imgMsg->SetDeviceName("Image");
    imgMsg->SetTimeStamp(sec, frac);
    imgMsg->SetOrigin(0, 0, 0);
    // 8UC3彩色图像
    imgMsg->SetDimensions(frame.cols, frame.rows, 1);
    imgMsg->SetSpacing(1.0, 1.0, 1.0);
    imgMsg->SetScalarType(igtl::ImageMessage::TYPE_UINT8);
    imgMsg->SetNumComponents(3);
    imgMsg->SetSubVolume(frame.cols, frame.rows, 1, 0, 0, 0);
    imgMsg->AllocateScalars();
    memcpy(imgMsg->GetScalarPointer(), frame.data, frame.cols*frame.rows*3);
    imgMsg->Pack();


    // 文本
    igtl::StringMessage::Pointer stringMsg = igtl::StringMessage::New();
    stringMsg->SetDeviceName("PoseStr");
    stringMsg->SetTimeStamp(sec, frac);
    stringMsg->SetString(msg_part1.c_str());
    stringMsg->Pack();

    // 发送
    int r = this->openIGLLinkSocket->Send(stringMsg->GetPackPointer(), stringMsg->GetPackSize());
    qDebug()<<"==="<<r;
    r = this->openIGLLinkSocket->Send(imgMsg->GetPackPointer(), imgMsg->GetPackSize());
    qDebug()<<r;
    return -1;
}


#endif // EXPORTOPENIGTLINK_H
