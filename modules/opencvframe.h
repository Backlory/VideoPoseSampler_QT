#ifndef OPENCVFRAME_H
#define OPENCVFRAME_H

#include <string>
#include <assert.h>
#include "opencv2/opencv.hpp"
#include <boost/thread.hpp>

class OpenCVFrame {
public:

    static OpenCVFrame& GetInstance()
    {
        static OpenCVFrame ocvf;
        return ocvf;
    }
    //
    bool Open(const int);
    bool Close();
    bool isOpened();
    cv::Mat getFrame();
    std::string getInfo();
private:

    OpenCVFrame();
    OpenCVFrame(const OpenCVFrame&) = delete;
    ~OpenCVFrame();

    cv::VideoCapture* cap;
    bool _Reset();
    cv::Mat frame_inflow;
    cv::Mat frame_out;

    int onRunning();
    std::shared_ptr<boost::thread> m_thread;
    mutable std::mutex m_lock;
};

#endif // OPENCVFRAME_H
