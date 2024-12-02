/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-03-16 13:24:52
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-11-28 11:20:02
 * @Description: 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
#include "export_OpenIGTLink.h"

ExportOpenIGTLink::ExportOpenIGTLink()
{
}

ExportOpenIGTLink::~ExportOpenIGTLink()
{
}

/*
 * 必须在已经init后调用
*/
void ExportOpenIGTLink::getAddress(std::string & address){
    if(this->openIGLLinkSocket){
        int temp;
        this->openIGLLinkSocket->GetSocketAddressAndPort(address, temp);
        return;
    }
    return;
}

using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;
//将QuatTransformationStruct转换为QString
template<>
void ndiData2QStringOpenIGTLink<data_ptr7>(data_ptr7 mat, QString & str){
    auto d2s = [](double d){return QString::number(d);};
    if(mat){
        str = "[" + d2s(mat->translation.x) + ", " + d2s(mat->translation.y) + ", " + d2s(mat->translation.z) +
            + ", " + d2s(mat->rotation.q0) + ", " + d2s(mat->rotation.qx) + ", " + d2s(mat->rotation.qy) + ", " + d2s(mat->rotation.qz) + "]";
    }
}

bool ExportOpenIGTLink::Init(int port){
    //先检查是否已经存在，防止内存泄漏
    if(this->openIGLLinkServer){
        this->openIGLLinkServer->CloseSocket();
        igtl::Sleep(100);
        this->openIGLLinkServer = nullptr;
        this->openIGLLinkSocket = nullptr;
    }
    // 创建
    this->openIGLLinkServer = igtl::ServerSocket::New();
    if (this->openIGLLinkServer.IsNull()) {
        return false;
    }
    int r = this->openIGLLinkServer->CreateServer(port);
    if (r < 0) {
        std::cerr << "Cannot create a server socket." << std::endl;
        return false;
    }
    //打印ip\port情况
    int temp_port;
    std::string temp_address;
    this->openIGLLinkServer->
            GetSocketAddressAndPort(temp_address, temp_port);
    std::cout << "openIGTLink server open at: " << temp_address << ":" << temp_port;
    //等待连接
    int retryCount = 0;
    while (this->openIGLLinkSocket.IsNull()){
        this->openIGLLinkSocket = this->openIGLLinkServer->WaitForConnection(1000);
        qDebug()<<"wait for connection..."<<retryCount;
        retryCount++;
        if (retryCount>60){
            return false;
        }
    }
    return true;
}


bool ExportOpenIGTLink::Close(){
    if(this->openIGLLinkServer){
        this->openIGLLinkServer->CloseSocket();
        igtl::Sleep(100);
        this->openIGLLinkServer = nullptr;
        this->openIGLLinkSocket = nullptr;
    }
    return true;
}
