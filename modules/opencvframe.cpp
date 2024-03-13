/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-03-12 14:06:59
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-03-13 22:09:19
 * @Description: 
 * 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
#include "opencvframe.h"

OpenCVFrame::OpenCVFrame() {
    this->cap = nullptr;
    this->m_thread = std::make_shared<boost::thread>(std::bind(&OpenCVFrame::onRunning, this));
}

OpenCVFrame::~OpenCVFrame()
{
    if (m_thread) { 
        m_thread->interrupt(); // 中断线程
        m_thread->try_join_for(boost::chrono::milliseconds(200)); // 等待线程结束
        //
        if (this->cap != nullptr) {
            if(this->cap->isOpened()){
                this->cap->release();
            }
        }
    }
}

int OpenCVFrame::onRunning(){
    while(!boost::this_thread::interruption_requested()){  // 线程未被中断
        if (this->cap != nullptr){
            if (this->cap->isOpened()){
                this->cap->read(this->frame_inflow);
            }
        }
        boost::this_thread::interruption_point(); // 线程中断点
    }
    return 0;
}

/*
 * 关闭并重置cap为nullptr
*/
bool OpenCVFrame::_Reset(){
    if (this->cap != nullptr) {
        if (this->cap->isOpened()) {
            this->cap->release();
        }
        this->cap = nullptr;
    }
    assert(this->cap == nullptr);
    return true;
}

/*
* 当cap为nullptr时，打开视频流，否则报错
*/
bool OpenCVFrame::Open(const int index) {
    if (this->cap != nullptr){
        throw "video capture has been opened!";
    }
    this->cap = new cv::VideoCapture(index);
    return this->cap->isOpened();
}

bool OpenCVFrame::isOpened(){
    if (this->cap == nullptr)
        return false;
    else
        return this->cap->isOpened();
}


bool OpenCVFrame::Close(){
    this->_Reset();
    return true;
}

/*
 * 获取视频流的信息
*/
std::string OpenCVFrame::getInfo(){
    if (this->cap == nullptr){
        throw "video capture has not been opened!";
    }
    std::string info;
    info += "index: " + std::to_string(this->cap->get(cv::CAP_PROP_POS_FRAMES)) + "\n";
    info += "channals: " + std::to_string(this->cap->get(cv::CAP_PROP_CHANNEL)) + "\n";
    info += "width: " + std::to_string(this->cap->get(cv::CAP_PROP_FRAME_WIDTH)) + "\n";
    info += "height: " + std::to_string(this->cap->get(cv::CAP_PROP_FRAME_HEIGHT)) + "\n";
    info += "fps: " + std::to_string(this->cap->get(cv::CAP_PROP_FPS)) + "\n";
    return info;
}

/*
 * 获取视频流的帧。加锁截取一帧，转换为RGB格式并返回
*/
cv::Mat OpenCVFrame::getFrame(){
    {
        std::lock_guard<std::mutex> _(m_lock);
        cv::cvtColor(this->frame_inflow, this->frame_out, cv::COLOR_BGR2RGB);
        return this->frame_out;
    }
}
