#ifndef NDIMODULE_H
#define NDIMODULE_H

#include <vector>
#include "../3rdparty/Aurora/INIFileRW.h"
#include "../3rdparty/Aurora/CommandHandling.h"
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

using data_ptr = std::shared_ptr<QuatTransformationStruct>;

class NDIModule {
public:
    // NDI device init
    typedef enum : int {
        NdiClosed = 0,
        NdiReseted,
        NdiActvied,

        NdiUnTracking = NdiActvied,
        NdiTracking,
    } State_t; // NDI状态编码，包括关闭、重置、激活、未跟踪、跟踪

public:
    static NDIModule& GetInstance()
    {
        static NDIModule device;
        return device;
    }
    bool Initialize(bool forceReset = false);
    bool Open();
    bool Close();
    bool isOpened();

    std::vector<int> getHandlers() const;
    bool getPosition(std::map<int, data_ptr> &positions) const;
    boost::signals2::signal<void(int, QuatTransformationStruct)> OnUpdate;

private:
    NDIModule();
    ~NDIModule();
    NDIModule(const NDIModule&) = delete;

    std::shared_ptr<boost::thread> m_thread;
    int running();

    std::shared_ptr<CCommandHandling> m_aurora;
    std::atomic<int> m_ports = -1;
    std::atomic<State_t> m_state = { NdiClosed };
    std::map<int, data_ptr> m_data;
    mutable std::mutex m_lock;

    std::vector<int> handles;
};

#endif // NDIMODULE_H
