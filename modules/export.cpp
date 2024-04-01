/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-03-16 13:24:52
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-04-01 18:47:54
 * @Description: 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
#include "export.h"

Export::Export()
{

}

Export::~Export()
{

}




// =======================

using data_ptr4 = std::shared_ptr<Eigen::Matrix4d>;
using data_ptr6 = std::shared_ptr<Sophus::Vector6d>;
using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;


//将Eigen::Matrix4d转换为QString
//[[a11, a12, a13, a14]\n, [a21, a22, a23, a24]\n, [a31, a32, a33, a34]\n, [a41, a42, a43, a44]]\n
template<>
void ndiData2QString<data_ptr4>(data_ptr4 mat, QString & str){
    auto i2s = [&mat](int i, int j){return QString::number((*mat)(i,j));};
    if(mat){
        str += "[[" + i2s(0,0) + ", " + i2s(0,1) + ", " + i2s(0,2) + ", " + i2s(0,3) + "],\n";
        str += "[" + i2s(1,0) + ", " + i2s(1,1) + ", " + i2s(1,2) + ", " + i2s(1,3) + "],\n";
        str += "[" + i2s(2,0) + ", " + i2s(2,1) + ", " + i2s(2,2) + ", " + i2s(2,3) + "],\n";
        str += "[" + i2s(3,0) + ", " + i2s(3,1) + ", " + i2s(3,2) + ", " + i2s(3,3) + "]]";
    }
}
//将Sophus::Vector6d转换为QString
template<>
void ndiData2QString<data_ptr6>(data_ptr6 mat, QString & str){
    auto i2s = [&mat](int i){return QString::number((*mat)(i));};
    if(mat){
        str += "[" + i2s(0) + ", " + i2s(1) + ", " + i2s(2) + ", " + i2s(3) + ", " + i2s(4) + ", " + i2s(5) + "]";
    }
}

//将QuatTransformationStruct转换为QString
template<>
void ndiData2QString<data_ptr7>(data_ptr7 mat, QString & str){
    auto d2s = [](double d){return QString::number(d);};
    if(mat){
        str = "[" + d2s(mat->translation.x) + ", " + d2s(mat->translation.y) + ", " + d2s(mat->translation.z) +
            + ", " + d2s(mat->rotation.q0) + ", " + d2s(mat->rotation.qx) + ", " + d2s(mat->rotation.qy) + ", " + d2s(mat->rotation.qz) + "]";
    }
}

// =======================
void Export::createFolder(const QString savePath) const{
    QDir dir;
    if (!dir.exists(savePath)) {
        dir.mkpath(savePath);
        dir.mkpath(savePath + "/frame");
        dir.mkpath(savePath + "/PoseWithTimeStamp");
    }
}

void Export::saveFrame(const QString frameIndexStr, const cv::Mat frame, const QString savePath) const{
    QString filePath = savePath + "/frame/" + frameIndexStr + ".jpg";
    cv::imwrite(filePath.toStdString(), frame);
}

/*
 * d7长度不定，若为某键的值为空，则d4对应值也为空。
*/
void Export::d7to4(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr4> &d4) {
    d4.clear();
    for (auto it = d7.begin(); it != d7.end(); it++) {
        auto id = it->first;
        auto ptr = it->second;
        if (ptr) {
            Eigen::Vector3d pos(ptr->translation.x, ptr->translation.y, ptr->translation.z);
            Eigen::Quaterniond rot(ptr->rotation.q0, ptr->rotation.qx, ptr->rotation.qy, ptr->rotation.qz);
            Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
            transformation.block<3,3>(0,0) = rot.toRotationMatrix();
            transformation.block<3,1>(0,3) = pos;
            auto ptr4 = std::make_shared<Eigen::Matrix4d>(transformation);
            d4.emplace(id, ptr4);
        }
        else {
            d4.emplace(id, nullptr);
        }
    }
}

/*
 * d7长度不定，若为某键的值为空，则d6对应值也为空。
*/
void Export::d7to6(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr6> &d6) {
    d6.clear();
    for (auto it = d7.begin(); it != d7.end(); it++) {
        auto id = it->first;
        auto ptr = it->second;
        if (ptr) {
            try{
                Eigen::Vector3d pos(ptr->translation.x, ptr->translation.y, ptr->translation.z);
                Eigen::Quaterniond rot(ptr->rotation.q0, ptr->rotation.qx, ptr->rotation.qy, ptr->rotation.qz);
                rot.normalize();
                Eigen::Vector3d lie = Sophus::SO3d::SO3(rot.toRotationMatrix()).log();
                auto ptr6 = std::make_shared<Sophus::Vector6d>((Sophus::Vector6d() << pos, lie).finished());
                d6.emplace(id, ptr6);
            }
            catch(...){
                auto ptr6 = std::make_shared<Sophus::Vector6d>((Sophus::Vector6d() << 0, 0, 0, 0, 0, 0).finished());
                d6.emplace(id, ptr6);
            }
        }
        else {
            d6.emplace(id, nullptr);
        }
    }
}

/* ==================socket============*/
bool Export::SocketInit(std::string adddress , int port){
    WSAStartup(MAKEWORD(2, 2), &this->wsaData);
    /*------------创建服务器---------------*/
    this->sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    this->addrServer.sin_family = AF_INET;
    this->addrServer.sin_port = htons(port);
    this->addrServer.sin_addr.S_un.S_addr = inet_addr(adddress.c_str());
    bind(this->sockServer, (SOCKADDR*)&this->addrServer, sizeof(SOCKADDR));
    
    int nRecvBuf = 1024; //设置为1K
    setsockopt(this->sockServer, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
    int nSendBuf = 200 * 1024; //设置为200K
    setsockopt(this->sockServer, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
    //int nNetTimeout = 5;
    //setsockopt(this->sockServer, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));

    listen(this->sockServer, 2); //监听连接请求
    try {
        this->sockClient = accept(this->sockServer, NULL, NULL); //接受连接请求
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
            //关闭socket
            closesocket(this->sockServer);
            WSACleanup();
    }
    



    char recvBuf[100];
    if (recv(this->sockClient, recvBuf, sizeof(recvBuf), 0) == -1) {
        return false;
    }
    std::string recvMsg(recvBuf);
    if (recvMsg.compare("hellofromclient") != 0) {
        return false;
    }
    std::string sendMsg = "hellofromserver";
    if (send(this->sockClient, sendMsg.c_str(), sendMsg.size() + 1, 0) == -1) {
        return false;
    }
    return true;
    
    /*-------------------------------创建socket-----------------------------------*/
    /*
    memset(&this->addrServer, 0, sizeof(this->addrServer));  //每个字节都用0填充
    this->sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // SOCK_STREAM为TCP流式套接字，SOCK_DGRAM为UDP数据报套接字
    this->addrServer.sin_addr.S_un.S_addr = inet_addr(adddress.c_str());
    this->addrServer.sin_family = AF_INET;
    this->addrServer.sin_port = htons(port);
    int nRecvBuf = 1024; //设置为1K
    setsockopt(this->sockClient, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
    int nSendBuf = 200 * 1024; //设置为200K
    setsockopt(this->sockClient, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
    connect(this->sockClient, (SOCKADDR*)&this->addrServer, sizeof(SOCKADDR));
    */
   
    //缓冲区大小
    //连接
    
    // 测试是否连接成功
    /*
    std::string sendMsg = "hello";
    if (send(this->sockClient, sendMsg.c_str(), sendMsg.size() + 1, 0) == -1) {
        return false;
    }

    char recvBuf[6];
    if (recv(this->sockClient, recvBuf, sizeof(recvBuf), 0) == -1) {
        return false;
    }

    std::string recvMsg(recvBuf);
    if (recvMsg.compare("hello") == 0) {
        return true;
    } else {
        return false;
    }*/
}


bool Export::SocketClose(){
    closesocket(this->sockClient);
    closesocket(this->sockServer);
    WSACleanup();
    return true;
}
