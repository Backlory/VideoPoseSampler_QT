/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-03-12 14:06:59
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-11-18 17:48:27
 * @Description: 
 * 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
#include "opencvframe.h"

OpenCVFrame::OpenCVFrame() {
    this->cap = nullptr;
    this->th_sampleId = 0;
    this->th_timeStamp = "";
    this->m_thread = std::make_shared<boost::thread>(std::bind(&OpenCVFrame::onRunning, this)); //这里的m_thread是异常安全的
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
                //
                this->th_sampleId++;
                //this->th_timeStamp = "";
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
bool OpenCVFrame::Open(const int index, const int frameHeight, const int frameWidth) {
    if (this->cap != nullptr){
        throw "video capture has been opened!";
    }
    this->cap = new cv::VideoCapture;
    this->cap->open(index);
    if (frameHeight>0 && frameWidth>0){
        this->changeSize(frameHeight, frameWidth);
    }
    this->CaptureDeviceidx = index;
    this->th_sampleId = 0;
    this->th_timeStamp = "";
    return this->cap->isOpened();
}

bool OpenCVFrame::isOpened(){
    if (this->cap == nullptr)
        return false;
    else
        return this->cap->isOpened();
}

bool OpenCVFrame::changeSize(const int frameHeight, const int frameWidth){
    if (this->isOpened()){
        this->cap->set(cv::CAP_PROP_FRAME_HEIGHT, frameHeight);
        this->cap->set(cv::CAP_PROP_FRAME_WIDTH, frameWidth);
        return true;
    }
    return false;
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
 * 时间：read指令时刻，真正成像时刻，流中保存时刻，取得数据时刻。
 * 在没有GPS授时的情况下，绝对时间都是不准的，这里的时间戳只是为了相对同步，因此不论是其实放哪里都行。
 * return True: success； false：failed
*/
bool OpenCVFrame::getFrame(cv::Mat &fm, cv::Rect &clip_roi){
    {
        double t = HSTime::wall_time(); // 取得数据时刻
        std::lock_guard<std::mutex> _(m_lock);
        this->frame_inflow.copyTo(fm);
        this->timeStamp_inflow = t; //
    }
    if (clip_roi.width <= 0) { // 无需roi处理
        return true;
    }
    else{
        this->clipROI = clip_roi;
        cv::Size fmSize = fm.size();
        if (clip_roi.x < 0 || clip_roi.y < 0 ||
            clip_roi.x + clip_roi.width > fmSize.width || clip_roi.y + clip_roi.height > fmSize.height) {
            fm = cv::Mat();
            return false;
        }
        else{
            fm = fm(clip_roi);
            return true;
        }
    }
}
