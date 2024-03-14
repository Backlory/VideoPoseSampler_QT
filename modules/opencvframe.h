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
    bool getFrame(cv::Mat &);
    std::string getInfo();
private:

    OpenCVFrame();
    OpenCVFrame(const OpenCVFrame&) = delete;
    ~OpenCVFrame();

    cv::VideoCapture* cap;
    bool _Reset();
    cv::Mat frame_inflow;

    int onRunning();
    std::shared_ptr<boost::thread> m_thread;
    mutable std::mutex m_lock;
};

#endif // OPENCVFRAME_H
