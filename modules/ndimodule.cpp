#include "ndimodule.h"

NDIModule::NDIModule()
{
    this->comPort = -1;
}

NDIModule::~NDIModule() {
    if (this->m_thread) {
        this->m_thread->interrupt();  //触发子线程中断
        this->m_thread->try_join_for(boost::chrono::milliseconds(200));
    }
}


bool NDIModule::Initialize(bool forceReset, int comPort) {
    if (this->m_state >= NdiReseted) {
        if (forceReset) {
            this->Close();
        }
        else {
            return false;
        }
    }
        emit progressUpdate(1, "[1/17]开始初始化...");
    auto ccmd = std::make_shared<CCommandHandling>();
    m_aurora.swap(ccmd);
    m_ports = -1;
    //
    if(this->m_state == NdiTracking) {
        emit progressUpdate(5, "[2/17]检测到跟踪状态，正在停止跟踪...");
        m_aurora->nStopTracking();
    }
    this->m_state = NdiClosed;

    // load ini
    emit progressUpdate(10, "[3/17]读取设置：硬件重置...");
    bool bResetHardware = false;
    ReadINIParm("Communication", "Reset Hardware", "0", &bResetHardware);

    emit progressUpdate(15, "[4/17]读取设置：端口号...");
    int nComPort = 0;
    if (comPort > 0) {
        nComPort = comPort;
        this->comPort = comPort;
    }
    else
        ReadINIParm("Communication", "COM Port", "0", &nComPort);

    // When reseted, and the NDI baudrate is to be changed, should be 9600 for communication.
    emit progressUpdate(20, "[5/17]连接NDI：打开端口...");
    if (!m_aurora->nOpenComPort(nComPort)) {
        throw std::runtime_error("COM Port could not be opened! Check if NDI Aurora is connected.");
    }

    if (bResetHardware) { //time cost a lot
        emit progressUpdate(25, "[6/17]连接NDI：试图发起硬件重置...");
        if (!m_aurora->nHardWareReset()) {
            throw std::runtime_error("HardWareReset Error_nHardWareReset!");
        }
        emit progressUpdate(30, "[7/17]连接NDI：重置后的硬件初始化...");
        if (!m_aurora->nSetCompCommParms(0, 0, 0, 0, 0)) {
            throw std::runtime_error("HardWareReset Error_nSetCompCommParms!");
        }
    }
    emit progressUpdate(35, "[8/17]读取设置：硬件参数...");
    int nBaudRate = 0, nStopBits = 0, nParity = 0, nDataBits = 0, nHardware = 0;
    ReadINIParm("Communication", "Baud Rate", "0", &nBaudRate); // 波特率
    ReadINIParm("Communication", "Stop Bits", "0", &nStopBits);
    ReadINIParm("Communication", "Parity", "0", &nParity);
    ReadINIParm("Communication", "Data Bits", "0", &nDataBits);
    ReadINIParm("Communication", "Hardware", "0", &nHardware);
    qDebug()<<nBaudRate<<nStopBits<<nParity<<nDataBits<<nHardware;

    emit progressUpdate(40, "[9/17]连接NDI：设置硬件参数 到 NDI Aurora系统...");
    if (!m_aurora->nSetSystemComParms(nBaudRate, nDataBits, nParity, nStopBits, nHardware)) {
        throw std::runtime_error("System Reset failed");
    }
    emit progressUpdate(45, "[10/17]连接NDI：设置硬件参数 到 上位机串口...");
    if (!m_aurora->nSetCompCommParms(nBaudRate, nDataBits, nParity, nStopBits, nHardware)) {
        throw std::runtime_error("System Reset failed");
    }
    emit progressUpdate(50, "[11/17]连接NDI：系统初始化...");
    if (!m_aurora->nInitializeSystem()) {
        throw std::runtime_error("System initialize failed");
    }
    emit progressUpdate(55, "[12/17]连接NDI：获取系统信息...");
    if (!m_aurora->nGetSystemInfo()) {
        throw std::runtime_error("Could not determine type of system");
    }
    emit progressUpdate(60, "[13/17]连接NDI：释放、初始化、激活全部端口...");
    if (!m_aurora->nActivateAllPorts()) { // 此后灯会变红
        throw std::runtime_error("Ports could not be activated.");
    }

    // 获取Handles，长度为4
    emit progressUpdate(70, "[14/17]数据处理：获取成功激活的端口数目...");
    this->handles.clear();
    this->m_ports = this->m_aurora->m_dtSystemInformation.nNoMagneticPorts;
    char pszPortID[32];

    emit progressUpdate(80, "[15/17]数据处理：为每个有效端口分配编号...");
    for (int i = 0; i < m_ports; i++) {
        sprintf_s(pszPortID, "%02d", i + 1);
        auto id = this->m_aurora->nGetHandleForPort(pszPortID);
        if (id != 0)
            this->handles.push_back(id);
        else
            this->handles.push_back(-1);
    }

    emit progressUpdate(90, "[16/17]数据处理：初始化位姿数据...");
    // 构造字典：端口号->矩阵。互斥锁确保端口数目不会变化。
    {
        std::lock_guard<std::mutex> _(this->m_lock);
        m_data.clear();
        for (auto id:this->handles) {
            if (id > 0){
                auto ptr = std::make_shared<QuatTransformationStruct>();
                m_data.emplace(id, ptr);
            }
        }
    }
    emit progressUpdate(100, "[17/17]连接完毕，NDI设备已经成功激活。");
    m_state = NdiActvied;
    return true;
}

bool NDIModule::Open() {
    if (m_state >= NdiTracking) {
        return true;
    }
    if (m_state <= NdiReseted) {
        throw std::runtime_error("Device not ready.");
    }
    if (m_state == NdiTracking) {
        throw std::runtime_error("Device already opened.");
    }
    if (!m_aurora->nStartTracking()) {
        throw std::runtime_error("Could not open tracking mode.");
    }
    try {
        m_thread = std::make_shared<boost::thread>(std::bind(&NDIModule::running, this));
    } catch (const std::exception& e) {
        throw std::runtime_error("Could not start sensor aquire thread sucessfully. please reopen");
    }
    m_state = NdiTracking;
    return true;
}

bool NDIModule::isOpened(){
    return (this->m_state == NdiTracking);
}

bool NDIModule::StopTracking() {
    if (this->m_state <= NdiUnTracking) {
        return true;
    }
    if (this->m_thread) {
        this->m_thread->interrupt();
        this->m_thread->join();
        this->m_thread.reset();
    }
    if (this->m_state <= NdiReseted) {
        throw std::runtime_error("Device not ready.");
    }
    if (this->m_state == NdiUnTracking) {
        throw std::runtime_error("Device already closed.");
    }
    if (this->m_state == NdiUnTracking || !m_aurora->nStopTracking()) {
        throw std::runtime_error("Tracking could not be stoped.");
    }
    this->m_state = NdiUnTracking;
    return true;
}

bool NDIModule::Close() {
    if (this->comPort > 0){
        try{
            this->StopTracking();
            m_aurora->nBeepSystem(1);
            m_aurora->nCloseComPorts();
        } catch (std::exception &e){
            return false;
        }
    }
    return true;
}


/*
 * 遍历编号0123，输出id。
*/
std::vector<int> NDIModule::getHandlers() const {
    return this->handles;
}

/*
 * 输出四个向量，全-1代表没开开。
 * std::vector<int> h = NDIModule::getHandlers();
 * std::map<int, data_ptr7> p;
 * NDIModule::getPosition(p);
 * auto out1 = p[h[0]];
 */
bool NDIModule::getPosition(std::map<int, data_ptr7> &positions) const {
    {
        std::lock_guard<std::mutex> _(this->m_lock);
        positions = this->m_data;
    }
    return true;
}

int NDIModule::running() {
    while (!boost::this_thread::interruption_requested()) {
        try{
            if (this->m_state == NdiTracking) {
                if(this->m_aurora->nGetTXTransforms(true)) {
                    // get data
                    std::map<int, QuatTransformationStruct> data;
                    for (auto ite = this->m_data.begin(); ite!= this->m_data.end(); ++ite) {
                        int i = ite->first;
                        if (this->m_aurora->m_dtHandleInformation[i].Xfrms.ulFlags == TRANSFORM_VALID) {
                            QuatTransformationStruct d;
                            auto& _pos = this->m_aurora->m_dtHandleInformation[i].Xfrms.translation;
                            auto& _rot = this->m_aurora->m_dtHandleInformation[i].Xfrms.rotation;
                            d.rotation = _rot;
                            d.translation = _pos;
                            data.emplace(i, d);
                        }
                        else
                        {
                            //throw std::runtime_error("不对啊");
                            QuatTransformationStruct d;
                            d.rotation = { 1,0,0,0 };
                            d.translation = { 0, 0, 0 };
                            data.emplace(i, d);
                        }
                    }
                    //to ensure all value in m_data are sampled at the same time.
                    {
                        std::lock_guard<std::mutex> _(this->m_lock);
                        for (auto& d : data) {
                            this->m_data.at(d.first) = std::make_shared<QuatTransformationStruct>(d.second);
                        }
                    }
                }
            }
            boost::this_thread::interruption_point();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        catch( const std::exception& e){
            std::cerr<<"ERROR in sensor thread:"<<e.what();
        }
    }
    return 0;
}
