#ifndef OPENCVFRAME_H
#define OPENCVFRAME_H

#include <string>
#include <assert.h>
#include "opencv2/opencv.hpp"
#include <boost/thread.hpp>
#include "highresolutiontime.h""

class OpenCVFrame {
public:

    static OpenCVFrame& GetInstance()
    {
        static OpenCVFrame ocvf;
        return ocvf;
    }
    //
    bool Open(const int, const int, const int );
    bool Close();
    bool isOpened();
    bool changeSize(const int, const int);
    bool getFrame(cv::Mat &, cv::Rect &);
    std::string getInfo();
private:

    OpenCVFrame();
    OpenCVFrame(const OpenCVFrame&) = delete;
    ~OpenCVFrame();

    int CaptureDeviceidx;
    cv::VideoCapture* cap;
    bool _Reset();
    cv::Mat frame_inflow;
    double timeStamp_inflow; // 时间戳
    cv::Rect clipROI;

    int onRunning();
    std::shared_ptr<boost::thread> m_thread;
    mutable std::mutex m_lock;
    long int th_sampleId;
    std::string th_timeStamp;

};

#endif // OPENCVFRAME_H
