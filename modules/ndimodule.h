#ifndef NDIMODULE_H
#define NDIMODULE_H

#include <QObject>
#include<QDebug>
#include <vector>
#include "Sophus/so3.hpp"
#include "../Aurora/INIFileRW.h"
#include "../Aurora/CommandHandling.h" // 在Sophus后导入
#include <boost/thread.hpp>
#include <Eigen/Dense>
#include <iostream>




class NDIModule:public QObject {
    Q_OBJECT
    using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

public:
    // NDI device init
    typedef enum : int {
        NdiClosed = 0,
        NdiReseted,
        NdiActvied,

        NdiUnTracking = NdiActvied,
        NdiTracking,
    } State_t; // NDI状态编码，包括关闭、重置、激活、未跟踪、跟踪
signals:
    void progressUpdate(int progress, const std::string &status);

public:
    static NDIModule& GetInstance()
    {
        static NDIModule device;
        return device;
    }
    bool Initialize(bool forceReset = false, int comPort = -1);
    bool Open();
    bool StopTracking();  //这里是，停止查询，但不关闭链接
    bool Close(); // 这里是关闭串口链接
    bool isOpened();

    std::vector<int> getHandlers() const;
    bool getPosition(std::map<int, data_ptr7> &positions, std::map<int, double> &errs) const;


private:
    NDIModule();
    ~NDIModule();
    NDIModule(const NDIModule&) = delete;

    std::shared_ptr<boost::thread> m_thread;
    int running();

    std::shared_ptr<CCommandHandling> m_aurora;
    std::atomic<int> m_ports = -1;
    std::atomic<State_t> m_state = { NdiClosed };
    std::map<int, data_ptr7> m_data; // 10:xx
    std::map<int, double> m_error; // 10:x
    mutable std::mutex m_lock;
    int comPort;

    std::vector<int> handles;
};

#endif // NDIMODULE_H
