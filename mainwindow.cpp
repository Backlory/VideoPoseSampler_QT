#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent, QString address, int port)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // instantiation
    timer = new QTimer(this);
    timer->setSingleShot(false);
    camDetImpl = new CameraDetection();
    comDetImpl = new COMPortDetection();

    modelCam = new QStandardItemModel(this);
    modelCom = new QStandardItemModel(this);

    DelayClibImpl = new DelayCalibration(this);
    TimeStampImpl = new TimeStamp();
    exptImpl = new Export();
    
    opcvFrmImpl = &OpenCVFrame::GetInstance();
    ndiImpl = &NDIModule::GetInstance();

    // socket
    this->sock_address = address.isEmpty()?"127.0.0.1" : address;
    this->sock_port = port == 0? 8998 : port;

    // connect
    connect(timer, &QTimer::timeout, this, &MainWindow::onTime);
    connect(ui->DelyaC_Pb, &QPushButton::clicked, this, &MainWindow::onDelyaC_PbClick);
    connect(ui->CamD_PbReset, &QPushButton::clicked, this, &MainWindow::onCamD_PbResetClick);
    connect(ui->CamD_PbConnect, &QPushButton::clicked, this, &MainWindow::onCamD_PbConnectClick);
    connect(ui->ComD_PbReset, &QPushButton::clicked, this, &MainWindow::onComD_PbResetClick);
    connect(ui->ComD_PbConnect, &QPushButton::clicked, this, &MainWindow::onComD_PbConnectClick);

    connect(ui->Export_PbRunPause, &QPushButton::clicked, this, &MainWindow::onExport_PbRunPauseClick);
    connect(ui->Export_Pb, &QPushButton::clicked, this, &MainWindow::onExport_PbClick);
    connect(ui->Export_PbSocket, &QPushButton::clicked, this, &MainWindow::onExport_PbSocketClick);


    connect(ui->NDI_Cb0, &QPushButton::clicked, this, &MainWindow::onNDI_Cb0Click);
    connect(ui->NDI_Cb1, &QPushButton::clicked, this, &MainWindow::onNDI_Cb1Click);
    connect(ui->NDI_Cb2, &QPushButton::clicked, this, &MainWindow::onNDI_Cb2Click);
    connect(ui->NDI_Cb3, &QPushButton::clicked, this, &MainWindow::onNDI_Cb3Click);

    // standby
    ui->CamD_PbReset->click();
    ui->ComD_PbReset->click();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete DelayClibImpl;
    delete TimeStampImpl;
    delete camDetImpl;
    delete exptImpl;
}

// ==================== ui交互部分 ============================================//
/*
    * 更新画面
    * @param frame: 画面帧
*/
void MainWindow::updateFrame(const cv::Mat & frame){

    cv::cvtColor(frame, cvframeDisp, cv::COLOR_BGR2RGB);
    QImage img = QImage((const unsigned char*)(cvframeDisp.data), cvframeDisp.cols, cvframeDisp.rows, QImage::Format_RGB888);
    QPixmap img2 = QPixmap::fromImage(img).scaled(ui->OpcvF_LableFrame->size(), Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->OpcvF_LableFrame->setPixmap(img2);
    QApplication::processEvents();
}

QString pose2str(QuatTransformationStruct * m){
    QString str;
    str += "x:" + QString::number(m->translation.x) + "\n";
    str += "y:" + QString::number(m->translation.y) + "\n";
    str += "z:" + QString::number(m->translation.z) + "\n";
    str += "q0:" + QString::number(m->rotation.q0) + "\n";
    str += "qx:" + QString::number(m->rotation.qx) + "\n";
    str += "qy:" + QString::number(m->rotation.qy) + "\n";
    str += "qz:" + QString::number(m->rotation.qz);
    return str;
}

void MainWindow::updatePose(const std::map<int, data_ptr7> poseMap){
    if (this->ndiActivated0 && this->ndiHandle[0]>0){
        auto m = poseMap.at(this->ndiHandle[0]);
        ui->NDI_LableOut0->setText(pose2str(m.get()));
    }
    if (this->ndiActivated1 && this->ndiHandle[1]>0){
        auto m = poseMap.at(this->ndiHandle[1]);
        ui->NDI_LableOut1->setText(pose2str(m.get()));
    }
    if (this->ndiActivated2 && this->ndiHandle[2]>0){
        auto m = poseMap.at(this->ndiHandle[2]);
        ui->NDI_LableOut2->setText(pose2str(m.get()));
    }
    if (this->ndiActivated3 && this->ndiHandle[3]>0){
        auto m = poseMap.at(this->ndiHandle[3]);
        ui->NDI_LableOut3->setText("NDI0: \n" + pose2str(m.get()));
    }
}

QString pose2str(Eigen::Matrix4d * m){
    QString str;
    auto i2s = [&m](int i, int j){return QString::number((*m)(i,j));};
    str += "T=" + i2s(0,3) + ", " + i2s(1,3) + ", " + i2s(2,3) + "\n";
    str += "R=" + i2s(0,0) + ", " + i2s(0,1) + ", " + i2s(0,2) + "\n";
    str += "   " + i2s(1,0) + ", " + i2s(1,1) + ", " + i2s(1,2) + "\n";
    str += "   " + i2s(2,0) + ", " + i2s(2,1) + ", " + i2s(2,2);
    return str;
}

void MainWindow::updatePose(const std::map<int, data_ptr4> poseMap){
    if (this->ndiActivated0 && this->ndiHandle[0]>0){
        auto m = poseMap.at(this->ndiHandle[0]);
        ui->NDI_LableOut0->setText(pose2str(m.get()));
    }
    if (this->ndiActivated1 && this->ndiHandle[1]>0){
        auto m = poseMap.at(this->ndiHandle[1]);
        ui->NDI_LableOut1->setText(pose2str(m.get()));
    }
    if (this->ndiActivated2 && this->ndiHandle[2]>0){
        auto m = poseMap.at(this->ndiHandle[2]);
        ui->NDI_LableOut2->setText(pose2str(m.get()));
    }
    if (this->ndiActivated3 && this->ndiHandle[3]>0){
        auto m = poseMap.at(this->ndiHandle[3]);
        ui->NDI_LableOut3->setText("NDI0: \n" + pose2str(m.get()));
    }
}

QString pose2str(Sophus::Vector6d * m){
    QString str;
    auto i2s = [&m](int i){return QString::number((*m)(i));};
    str += "x:" + i2s(0) + "\n";
    str += "y:" + i2s(1) + "\n";
    str += "z:" + i2s(2) + "\n";
    str += "rx:" + i2s(3) + "\n";
    str += "ry:" + i2s(4) + "\n";
    str += "rz:" + i2s(5);
    return str;
}

void MainWindow::updatePose(const std::map<int, data_ptr6> poseMap){
    if (this->ndiActivated0 && this->ndiHandle[0]>0){
        auto m = poseMap.at(this->ndiHandle[0]);
        ui->NDI_LableOut0->setText(pose2str(m.get()));
    }
    if (this->ndiActivated1 && this->ndiHandle[1]>0){
        auto m = poseMap.at(this->ndiHandle[1]);
        ui->NDI_LableOut1->setText(pose2str(m.get()));
    }
    if (this->ndiActivated2 && this->ndiHandle[2]>0){
        auto m = poseMap.at(this->ndiHandle[2]);
        ui->NDI_LableOut2->setText(pose2str(m.get()));
    }
    if (this->ndiActivated3 && this->ndiHandle[3]>0){
        auto m = poseMap.at(this->ndiHandle[3]);
        ui->NDI_LableOut3->setText("NDI0: \n" + pose2str(m.get()));
    }
}

/*
    * 主循环更新函数。包括：
    * 1. 时间戳更新
    * 2. 画面更新
    * 3. 位姿更新
    * 4. 数据导出
    * 5.
*/
void MainWindow::onTime(){
    // 时间戳更新
    ui->TimeS_Label->setText("当前时间戳: \t"+this->TimeStampImpl->getTimeStamp());

    // 同步触发
    if (this->opcvFrmImpl->isOpened()) {
        this->opcvFrmImpl->getFrame(this->cvframe);
    }
    if (this->ndiImpl->isOpened()){
        this->ndiData7.clear();
        this->ndiImpl->getPosition(this->ndiData7, this->DelayClibImpl->getFrameDelayMs()); //尺寸未知
    }

    QString details;
    details += "导出序号：" +  QString::number(this->exptIdx) + "\n";
    details += "导出位置：" + this->exptFolderPath + "\n";
    // 更新画面
    if (this->opcvFrmImpl->isOpened()) {
        this->updateFrame(this->cvframe);
        details += QString::fromStdString(opcvFrmImpl->getInfo());
    }

    // 更新位姿
    if (this->ndiImpl->isOpened()){
        switch(this->ndiOutputType){
        case 4:
            this->exptImpl->d7to4(ndiData7, ndiData4);
            this->updatePose(this->ndiData4);
            break;
        case 6:
            this->exptImpl->d7to6(ndiData7, ndiData6);
            this->updatePose(this->ndiData6);
            break;
        case 7:
            this->updatePose(this->ndiData7);
            break;
        }
        details += "电磁定位输出类型：" + QString::number(this->ndiOutputType) + "\n";
    }

    // 数据导出
    if (this->onExportingLocal || this->onExportingSocket) {

        this->ui->NDI_LableType0->setText("导出类型："+QString::number(this->ndiOutputType));
        this->ui->NDI_LableType1->setText("导出类型："+QString::number(this->ndiOutputType));
        this->ui->NDI_LableType2->setText("导出类型："+QString::number(this->ndiOutputType));
        this->ui->NDI_LableType3->setText("导出类型："+QString::number(this->ndiOutputType));
        if (this->onExportingLocal){
            switch(this->ndiOutputType){
            case 4:
                this->exptImpl->exportData<data_ptr4>(exptIdx, cvframe,
                                                      ndiHandle, ndiData4,
                                                      TimeStampImpl->getTimeStamp(), exptFolderPath);
                break;
            case 6:
                this->exptImpl->exportData<data_ptr6>(exptIdx, cvframe,
                                                      ndiHandle, ndiData6,
                                                      TimeStampImpl->getTimeStamp(), exptFolderPath);
                break;
            case 7:
                this->exptImpl->exportData<data_ptr7>(exptIdx, cvframe,
                                                      ndiHandle, ndiData7,
                                                      TimeStampImpl->getTimeStamp(), exptFolderPath);
                break;
            }
        }
        if (this->onExportingSocket){
            auto m = this->exptImpl->exportSocketData(exptIdx, cvframe,
                                                      ndiHandle, ndiData7,
                                                      TimeStampImpl->getTimeStamp());
            if(m > 0) {
                QMessageBox::warning(this, "错误", "客户端连接中断，错误代码="+QString::number(m));
                this->ui->Export_PbSocket->click();
                // 1-连接断开
                // 2-获取get失败
                // 3-发送
            }

        }
        details += "正在本地导出:" + QString(this->onExportingLocal?"true":"false") + "\n";
        details += "正在端口导出:" + QString(this->onExportingSocket?"true":"false") + "\n";
    }
    
    //std::vector<double> fpsList;
    details += "FPS:" + this->TimeStampImpl->getFPS();
    this->exptIdx++;
    ui->OpcvF_LableDetail->setText(details);
}
void MainWindow::onDelyaC_PbClick() {
    this->DelayClibImpl->doDelayCalib();
}

// ====== 01 camera 相关交互 ======


void MainWindow::onCamD_PbResetClick(){
    // 关闭播放
    if (this->onRunning == true){
        this->onRunning = false;
        this->timer->stop();
        this->ui->Export_PbRunPause->setText("启动");
    }
    // 清空
    modelCam->clear();
    QApplication::processEvents();
    // 关闭相机
    opcvFrmImpl->Close();
    ui->CamD_Label->setText("当前相机: 关闭");

    // 检查相机列表
    std::vector<std::string> camList = camDetImpl->detectCam();
    for(int i=0; i<camList.size(); i++){
        QStandardItem *item = new QStandardItem("camera:"+QString::fromStdString(camList.at(i)));
        modelCam->appendRow(item);
    }
    // 加载到列表
    ui->CamD_ListView->setModel(modelCam);
    ui->CamD_ListView->setCurrentIndex(modelCam->index(0, 0));
    QApplication::processEvents();
}

void MainWindow::onCamD_PbConnectClick(){
    opcvFrmImpl->Close();
    ui->CamD_Label->setText("当前相机: 关闭");
    //
    QModelIndex index = ui->CamD_ListView->currentIndex();
    int camIndex = index.row();
    camDetImpl->activateCam(std::to_string(camIndex));
    ui->CamD_Label->setText("当前相机: "+QString::fromStdString(camDetImpl->getActivateCam()));
    // 打开相机, str转int
    opcvFrmImpl->Open(camIndex);

    if (this->onRunning == false){
        this->onRunning = true;
        this->timer->start(10);
        this->ui->Export_PbRunPause->setText("暂停");
    }
}


// ======02 export 相关交互 ======
void MainWindow::onExport_PbRunPauseClick(){
    if (this->timer->isActive()){
        this->onRunning = false;
        this->timer->stop();
        this->ui->Export_PbRunPause->setText("启动");
        this->onExportingLocal = false;
        this->ui->Export_Pb->setText("开始采集（到本地）");
        this->ui->Export_Pb->setEnabled(false);
    }
    else{
        this->onRunning = true;
        this->timer->start(10); //FPS
        this->ui->Export_PbRunPause->setText("暂停");
        this->ui->Export_Pb->setEnabled(true);
    }
}

void MainWindow::onExport_PbClick() {
    if (this->onRunning){
        if (this->onExportingLocal == true) {
            this->onExportingLocal = false;
            this->ui->Export_Pb->setText("开始采集（到本地）");
            //this->ndiOutputType = -1;
        }
        else{
            bool bOK = false;
            int temp = QInputDialog::getInt(this, "导出类型", "4-四阶位姿矩阵\n6-欧拉角\n7-四元数",
                                                 7, 0, 7, 1, &bOK);
            if (bOK && (temp == 4||temp == 6|| temp == 7)){
                this->ndiOutputType = temp;
                this->onExportingLocal = true;
                this->ui->Export_Pb->setText("停止本地采集");
                //
                if(!this->onExportingSocket){  //确保都关闭才重置
                    this->exptIdx = 0;
                }
                this->exptFolderPath = "SampleOutput/" + this->TimeStampImpl->getTimeStamp();
                this->exptImpl->createFolder(exptFolderPath);
            }
        }
    }
}
void MainWindow::onExport_PbSocketClick() {
    if (this->onRunning){
        if (this->onExportingSocket == true) {
            this->onExportingSocket = false;
            this->ui->Export_PbSocket->setText("启动采集服务器");
            //this->ndiOutputType = -1;

            this->exptImpl->SocketClose();
        }
        else{
            bool bOK = false;
            int temp = QInputDialog::getInt(this, "导出类型", "4-四阶位姿矩阵\n6-欧拉角\n7-四元数",
                                                 7, 0, 7, 1, &bOK);
            if (bOK && (temp == 4||temp == 6|| temp == 7)){
                QString address = QInputDialog::getText(this, "地址", "请输入服务器开放的地址和端口（IP:Port）",
                    QLineEdit::Normal, this->sock_address + ":" + QString::number(this->sock_port), &bOK);
                if (bOK)
                {
                    //
                    std::string ip = address.split(":").at(0).toStdString();
                    int port = address.split(":").at(1).toInt();
                    this->ui->Export_PbSocket->setText("建立服务器...");
                    QApplication::processEvents();
                    if(this->exptImpl->SocketInit(ip, port)){
                        this->ndiOutputType = temp;
                        this->onExportingSocket = true;
                        this->ui->Export_PbSocket->setText("关闭采集服务器");
                        //
                        if(!this->onExportingLocal){  //确保都关闭才重置
                            this->exptIdx = 0;
                        }
                    }
                    else{
                        this->ui->Export_PbSocket->setText("启动采集服务器");
                        QMessageBox::warning(this, "错误", "无法启动服务器"+address);
                        this->exptImpl->SocketClose();
                    }
                }
            }
        }
    }
}

// ====== 03 COM口 交互 ======
void MainWindow::onComD_PbResetClick(){
    // 关闭播放
    if (this->onRunning == true){
        this->onRunning = false;
        this->timer->stop();
        this->ui->Export_PbRunPause->setText("启动");
    }
    // 清空
    modelCom->clear();
    QApplication::processEvents();
    // 关闭端口
    this->ndiImpl->Close();
    ui->ComD_Label->setText("当前端口: 关闭");

    // 检查相机列表
    std::vector<std::string> comList = comDetImpl->getAvailablePorts();
    for(int i=0; i<comList.size(); i++){
        QStandardItem *item = new QStandardItem(QString::fromStdString(comList.at(i)));
        modelCom->appendRow(item);
    }
    // 加载到列表
    ui->ComD_ListView->setModel(modelCom);
    ui->ComD_ListView->setCurrentIndex(modelCom->index(0, 0));
    QApplication::processEvents();
    return;
}
void MainWindow::onComD_PbConnectClick(){
    //清空
    if(this->ui->NDI_Cb0->isChecked()){this->ui->NDI_Cb0->click();}
    if(this->ui->NDI_Cb1->isChecked()){this->ui->NDI_Cb1->click();}
    if(this->ui->NDI_Cb2->isChecked()){this->ui->NDI_Cb2->click();}
    if(this->ui->NDI_Cb3->isChecked()){this->ui->NDI_Cb3->click();}
    // 关闭电磁定位
    if (this->ndiImpl->isOpened()){
        this->ndiImpl->Close();
    }
    // 读取
    std::string comName = ui->ComD_ListView->currentIndex().data().toString().toStdString();
    this->comDetImpl->activateCOM(comName);
    try {
        ndiImpl->Initialize(true, this->comDetImpl->getActivateCOM());
        ndiImpl->Open();
        this->ndiHandle = this->ndiImpl->getHandlers();
        assert(ndiHandle.size() == 4);
    }
    catch (std::runtime_error &e) {
        QMessageBox::warning(this, "错误", QString::fromStdString(e.what()));
        return;
    }
    // ui更新
    if (this->ndiHandle[0]>0) {
        this->ui->NDI_LableState0->setText("状态：connected");
        this->ui->NDI_Cb0->click();
    }
    else
    {this->ui->NDI_LableState0->setText("状态：disconnected");}
    if (this->ndiHandle[1]>0) {
        this->ui->NDI_LableState1->setText("状态：connected");
        this->ui->NDI_Cb1->click();
    }
    else
    {this->ui->NDI_LableState1->setText("状态：disconnected");}
    if (this->ndiHandle[2]>0) {
        this->ui->NDI_LableState2->setText("状态：connected");
        this->ui->NDI_Cb2->click();
    }
    else
    {this->ui->NDI_LableState2->setText("状态：disconnected");}
    if (this->ndiHandle[3]>0) {
        this->ui->NDI_LableState3->setText("状态：connected");
        this->ui->NDI_Cb3->click();
    }
    else
    {this->ui->NDI_LableState3->setText("状态：disconnected");}
    this->ui->ComD_PbConnect->setEnabled(false);
}

// ====== 04 NDI 交互 ======
void MainWindow::onNDI_Cb0Click(){
    this->ndiActivated0 = ui->NDI_Cb0->isChecked();
}
void MainWindow::onNDI_Cb1Click(){
    this->ndiActivated1 = ui->NDI_Cb1->isChecked();
}
void MainWindow::onNDI_Cb2Click(){
    this->ndiActivated2 = ui->NDI_Cb2->isChecked();
}
void MainWindow::onNDI_Cb3Click(){
    this->ndiActivated3 = ui->NDI_Cb3->isChecked();
}
