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

using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;
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
    struct timeval nNetTimeout;
    nNetTimeout.tv_sec = 5;  // 等待10秒
    nNetTimeout.tv_usec = 0;
    setsockopt(this->sockServer, SOL_SOCKET, SO_RCVTIMEO, (char*)&nNetTimeout, sizeof(nNetTimeout));

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
