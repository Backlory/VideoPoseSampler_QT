#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // instantiation
    timer = new QTimer(this);
    timer->setSingleShot(false);
    modelCam = new QStandardItemModel(this);

    DelayClibImpl = new DelayCalibration(this);
    TimeStampImpl = new TimeStamp();
    camDetImpl = new CameraDetection();
    opcvFrmImpl = &OpenCVFrame::GetInstance();
    ndiImpl = &NDIModule::GetInstance();

    // connect
    connect(timer, &QTimer::timeout, this, &MainWindow::onTime);
    connect(ui->DelyaC_Pb, &QPushButton::clicked, this, &MainWindow::onDelyaC_PbClick);
    connect(ui->CamD_PbReset, &QPushButton::clicked, this, &MainWindow::onCamD_PbResetClick);
    connect(ui->CamD_PbConnect, &QPushButton::clicked, this, &MainWindow::onCamD_PbConnectClick);
    connect(ui->ComD_PbReset, &QPushButton::clicked, this, &MainWindow::onComD_PbResetClick);
    connect(ui->ComD_PbConnect, &QPushButton::clicked, this, &MainWindow::onComD_PbConnectClick);

    connect(ui->Export_Pb_runPause, &QPushButton::clicked, this, &MainWindow::onExport_PbRunPauseClick);

    connect(ui->NDI_Rb0_6, &QPushButton::clicked, this, &MainWindow::onNDI_Rb0_6Click); // 选择6维
    connect(ui->NDI_Rb1_6, &QPushButton::clicked, this, &MainWindow::onNDI_Rb1_6Click);
    connect(ui->NDI_Rb2_6, &QPushButton::clicked, this, &MainWindow::onNDI_Rb2_6Click);
    connect(ui->NDI_Rb3_6, &QPushButton::clicked, this, &MainWindow::onNDI_Rb3_6Click);
    connect(ui->NDI_Rb0_4, &QPushButton::clicked, this, &MainWindow::onNDI_Rb0_4Click); // 选择4阶
    connect(ui->NDI_Rb1_4, &QPushButton::clicked, this, &MainWindow::onNDI_Rb1_4Click);
    connect(ui->NDI_Rb2_4, &QPushButton::clicked, this, &MainWindow::onNDI_Rb2_4Click);
    connect(ui->NDI_Rb3_4, &QPushButton::clicked, this, &MainWindow::onNDI_Rb3_4Click);
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
}

// ==================== ui交互部分 ============================================//
/*
    * 更新画面
    * @param frame: 画面帧
*/
void MainWindow::updateFrame(const cv::Mat frame){
    QImage img = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    QPixmap img2 = QPixmap::fromImage(img).scaled(ui->OpcvF_LableFrame->size(), Qt::KeepAspectRatio,Qt::SmoothTransformation);
    //QPixmap img2 = QPixmap::fromImage(img);
    ui->OpcvF_LableFrame->setPixmap(img2);
    QApplication::processEvents();
}

QString pose2str(QuatTransformationStruct * m){
    QString str;
    str += QString::number(m->translation.x) + ",";
    str += QString::number(m->translation.y) + ",";
    str += QString::number(m->translation.z) + "\n";
    str += QString::number(m->rotation.q0) + ",";
    str += QString::number(m->rotation.qx) + ",";
    str += QString::number(m->rotation.qy) + ",";
    str += QString::number(m->rotation.qz);
    return str;
}

void MainWindow::updatePose(const std::map<int, data_ptr> poseMap){
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
    // 更新画面
    cv::Mat cvframe;
    std::map<int, data_ptr> ndiData;
    if (this->opcvFrmImpl->isOpened()){
        this->opcvFrmImpl->getFrame(this->cvframe);
        this->updateFrame(this->cvframe);
        std::string details = opcvFrmImpl->getInfo();
        ui->OpcvF_LableDetail->setText(QString::fromStdString(details));
    }
    // 更新位姿
    if (this->ndiImpl->isOpened()){
        this->ndiImpl->getPosition(this->ndiData);
        this->updatePose(this->ndiData);
    }
    // 数据导出
}
void MainWindow::onDelyaC_PbClick() {
    this->DelayClibImpl->doDelayCalib();
    int frameDelayMs = this->DelayClibImpl->getFrameDelayMs();
    this->ui->OpcvF_LableDetail->setText(
        "延迟："+QString::number(frameDelayMs)+" ms"
        );
}

// ====== 01 camera 相关交互 ======


void MainWindow::onCamD_PbResetClick(){
    // 关闭播放
    if (this->onRunning == true){
        this->onRunning = false;
        this->timer->stop();
        this->ui->Export_Pb_runPause->setText("启动");
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
        this->ui->Export_Pb_runPause->setText("暂停");
    }
}


// ======02 export 相关交互 ======
void MainWindow::onExport_PbRunPauseClick(){
    if (this->timer->isActive()){
        this->onRunning = false;
        this->timer->stop();
        this->ui->Export_Pb_runPause->setText("启动");
    }
    else{
        this->onRunning = true;
        this->timer->start(1000/30);
        this->ui->Export_Pb_runPause->setText("暂停");
    }

}

// ====== 03 COM口 交互 ======
void MainWindow::onComD_PbResetClick(){
    // 清空COM口列表
    // 关闭电磁定位
    // 清空所有复选框
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
    ndiImpl->Initialize(true);
    ndiImpl->Open();
    this->ndiHandle = this->ndiImpl->getHandlers();
    assert(ndiHandle.size() == 4);
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
void MainWindow::onNDI_Rb0_6Click(){this->ndiOutputType0 = 6; return;}
void MainWindow::onNDI_Rb1_6Click(){this->ndiOutputType0 = 6; return;}
void MainWindow::onNDI_Rb2_6Click(){this->ndiOutputType0 = 6; return;}
void MainWindow::onNDI_Rb3_6Click(){this->ndiOutputType0 = 6; return;}
void MainWindow::onNDI_Rb0_4Click(){this->ndiOutputType0 = 4; return;}
void MainWindow::onNDI_Rb1_4Click(){this->ndiOutputType0 = 4; return;}
void MainWindow::onNDI_Rb2_4Click(){this->ndiOutputType0 = 4; return;}
void MainWindow::onNDI_Rb3_4Click(){this->ndiOutputType0 = 4; return;}
void MainWindow::onNDI_Cb0Click(){
    this->ndiActivated0 = ui->NDI_Cb0->isChecked();
    this->ui->NDI_Rb0_4->setEnabled(this->ndiActivated0);
    this->ui->NDI_Rb0_6->setEnabled(this->ndiActivated0);
}
void MainWindow::onNDI_Cb1Click(){
    this->ndiActivated1 = ui->NDI_Cb1->isChecked();
    this->ui->NDI_Rb1_4->setEnabled(this->ndiActivated1);
    this->ui->NDI_Rb1_6->setEnabled(this->ndiActivated1);
}
void MainWindow::onNDI_Cb2Click(){
    this->ndiActivated2 = ui->NDI_Cb2->isChecked();
    this->ui->NDI_Rb2_4->setEnabled(this->ndiActivated2);
    this->ui->NDI_Rb2_6->setEnabled(this->ndiActivated2);
}
void MainWindow::onNDI_Cb3Click(){
    this->ndiActivated3 = ui->NDI_Cb3->isChecked();
    this->ui->NDI_Rb3_4->setEnabled(this->ndiActivated3);
    this->ui->NDI_Rb3_6->setEnabled(this->ndiActivated3);
}
