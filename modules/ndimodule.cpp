#include "ndimodule.h"

NDIModule::NDIModule()
{

}

NDIModule::~NDIModule() {
    if (this->m_thread) {
        this->m_thread->interrupt();  //触发子线程中断
        this->m_thread->try_join_for(boost::chrono::milliseconds(200));
    }
}

bool NDIModule::Initialize(bool forceReset) {
    if (this->m_state >= NdiReseted) {
        if (forceReset) {
            this->Close();
        }
        else{
            return false;
        }
    }

    auto ccmd = std::make_shared<CCommandHandling>();
    m_aurora.swap(ccmd);
    m_ports = -1;
    //
    if(this->m_state == NdiTracking) {
        m_aurora->nStartTracking();
    }
    this->m_state = NdiClosed;

    // load ini
    bool bResetHardware = false;
    ReadINIParm("Communication", "Reset Hardware", "0", &bResetHardware);
    int nComPort = 0;
    ReadINIParm("Communication", "COM Port", "0", &nComPort);

    // When reseted, and the NDI baudrate is to be changed, should be 9600 for communication.
    if (!m_aurora->nOpenComPort(nComPort)) {
        throw std::runtime_error("COM Port could not be opened! Check if NDI Aurora is connected.");
    }
    if (bResetHardware) {
        if (!m_aurora->nHardWareReset()) {
            throw std::runtime_error("HardWareReset Error!");
        }
        if (!m_aurora->nSetCompCommParms(0, 0, 0, 0, 0)) {
            throw std::runtime_error("HardWareReset Error!");
        }
    }
    int nBaudRate = 0, nStopBits = 0, nParity = 0, nDataBits = 0, nHardware = 0;
    ReadINIParm("Communication", "Baud Rate", "0", &nBaudRate);
    ReadINIParm("Communication", "Stop Bits", "0", &nStopBits);
    ReadINIParm("Communication", "Parity", "0", &nParity);
    ReadINIParm("Communication", "Data Bits", "0", &nDataBits);
    ReadINIParm("Communication", "Hardware", "0", &nHardware);

    if (!m_aurora->nSetSystemComParms(nBaudRate, nDataBits, nParity, nStopBits, nHardware)) {
        throw std::runtime_error("System Reset failed");
    }
    if (!m_aurora->nSetCompCommParms(nBaudRate, nDataBits, nParity, nStopBits, nHardware)) {
        throw std::runtime_error("System Reset failed");
    }
    if (!m_aurora->nInitializeSystem()) {
        throw std::runtime_error("System initialize failed");
    }
    if (!m_aurora->nGetSystemInfo()) {
        throw std::runtime_error("Could not determine type of system");
    }
    if (!m_aurora->nActivateAllPorts()) {
        throw std::runtime_error("Ports could not be activated.");
    }
    // 获取Handles
    std::vector<int> ids;
    {
        this->m_ports = this->m_aurora->m_dtSystemInformation.nNoMagneticPorts;
        char pszPortID[32];
        for (int i = 0; i < m_ports; i++) {
            sprintf_s(pszPortID, "%02d", i + 1);
            auto id = this->m_aurora->nGetHandleForPort(pszPortID);
            if (id != 0)
                ids.push_back(id);
        }
    }
    // 构造字典：端口号->矩阵。互斥锁确保端口数目不会变化。
    {
        std::lock_guard<std::mutex> _(this->m_lock);
        m_data.clear();
        for (auto id:ids){
            auto ptr = std::make_shared<QuatTransformationStruct>();
            m_data.emplace(id, ptr);
        }
    }
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
        throw std::runtime_error("Tracking could not be started.");
    }
    m_state = NdiTracking;
    m_thread = std::make_shared<boost::thread>(std::bind(&NDIModule::running, this));
    return true;
}

bool NDIModule::Close() {
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

std::vector<int> NDIModule::getHandlers() const {
    std::vector<int> hs(m_data.size());
    std::transform(m_data.begin(), m_data.end(), hs.begin(), [](auto& a) { return a.first; });
    return hs;
}


std::vector<QuatTransformationStruct> NDIModule::getPosition() const{
    std::vector<QuatTransformationStruct> positions;
    {
        std::lock_guard<std::mutex> _(this->m_lock);
        for (const auto& pair : this->m_data) {
            positions.push_back(*pair.second); //TODO
        }
    }
    return positions;
}

int NDIModule::running(){
    while(!boost::this_thread::interruption_requested()){
        if (this->m_state == NdiTracking){
            if(this->m_aurora->nGetBXTransforms(true)){
                // get data
                std::map<int, QuatTransformationStruct> data;
                for (auto ite = this->m_data.begin(); ite!= this->m_data.end(); ++ite){
                    int i = ite->first;
                    if (this->m_aurora->m_dtHandleInformation[i].Xfrms.ulFlags == TRANSFORM_VALID){
                        auto& _pos = this->m_aurora->m_dtHandleInformation[i].Xfrms.translation;
                        auto& _rot = this->m_aurora->m_dtHandleInformation[i].Xfrms.rotation;
                        QuatTransformationStruct d;
                        d.rotation = _rot;
                        d.translation = _pos;
                        data.emplace(i, d);
                    }
                }
                //to ensure all value in m_data are sampled at the same time.
                {
                    std::lock_guard<std::mutex> _(this->m_lock);
                    for (auto& d : data) {
                        this->m_data.at(d.first)=std::make_shared<QuatTransformationStruct>(d.second);
                    }
                }
            }
        }
        boost::this_thread::interruption_point();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}
