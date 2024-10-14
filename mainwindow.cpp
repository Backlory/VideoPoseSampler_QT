#include "mainwindow.h"
#include "ui_mainwindow.h"

#define _MAJOR_VERSION_ "1"
#define _MINOR_VERSION_ "3"
#define _PATCH_VERSION_ "0"


MainWindow::MainWindow(QWidget *parent, QString address, int port, QString saveDir, QStringList roiValues, QStringList sizeValues)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(QString("VPS: VideoPoseSampler[" _MAJOR_VERSION_ "." _MINOR_VERSION_ "." _PATCH_VERSION_ "]"));

    // instantiation
    timer = new QTimer(this);
    timer->setSingleShot(false);
    camDetImpl = new CameraDetection();
    comDetImpl = new COMPortDetection();

    modelCam = new QStandardItemModel(this);
    modelCom = new QStandardItemModel(this);

    TimeStampImpl = new TimeStamp();
    exptImpl = new Export();
    exptPath = saveDir;
    
    opcvFrmImpl = &OpenCVFrame::GetInstance();
    ndiImpl = &NDIModule::GetInstance();

    // socket
    this->sock_address = address.isEmpty()?"127.0.0.1" : address;
    this->sock_port = port == 0? 8998 : port;
    // ROI
    if(roiValues.length() == 0){
        this->clipROI.x = 0;
        this->clipROI.y = 0;
        this->clipROI.width = 0;
        this->clipROI.height = 0;
    }
    else{
        try {
            auto roiValuesList = roiValues.at(0).split(",");
            this->clipROI.x = roiValuesList.at(0).toInt();
            this->clipROI.y = roiValuesList.at(1).toInt();
            this->clipROI.width = roiValuesList.at(2).toInt();
            this->clipROI.height = roiValuesList.at(3).toInt();
        } catch (...) {
            this->clipROI.x = 0;
            this->clipROI.y = 0;
            this->clipROI.width = 0;
            this->clipROI.height = 0;
        }
    }
    //size
    if(sizeValues.length() == 0){
        this->frameHeight = -1;
        this->frameWidth = -1;
    }
    else{
        try {
            auto sizeValuesList = sizeValues.at(0).split(",");
            this->frameHeight = sizeValuesList.at(0).toInt();
            this->frameWidth = sizeValuesList.at(1).toInt();
        } catch (...) {
            this->frameHeight = -1;
            this->frameWidth = -1;
        }
    }
    // progress
    QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Ignored);
    sizePolicy2.setHorizontalStretch(2);
    sizePolicy2.setVerticalStretch(0);

    progressBar = new QProgressBar(this);
    progressBar->setMinimum(0);
    progressBar->setMaximum(100);
    progressBar->setSizePolicy(sizePolicy2);
    progressBar->setVisible(false);
    ui->statusbar->addWidget(progressBar);

    progressLabel = new QLabel(this);
    progressLabel->setSizePolicy(sizePolicy2);
    progressLabel->setText("Ready.");
    ui->statusbar->addWidget(progressLabel);


    // connect
    connect(timer, &QTimer::timeout, this, &MainWindow::onTime);
    connect(ui->CamD_PbConnect, &QPushButton::clicked, this, &MainWindow::onCamD_PbConnectClick);
    connect(ui->CamD_PbChangeR, &QPushButton::clicked, this, &MainWindow::onCamD_PbChangeRClick);
    connect(ui->ComD_PbConnect, &QPushButton::clicked, this, &MainWindow::onComD_PbConnectClick);
    connect(ui->ComD_PbReset, &QPushButton::clicked, this, &MainWindow::onComD_PbResetClick);

    connect(ui->pB_showSave, &QPushButton::clicked, this, &MainWindow::onPbShowSaveClick);

    connect(ui->Export_PbRunPause, &QPushButton::clicked, this, &MainWindow::onExport_PbRunPauseClick);
    connect(ui->Export_Pb, &QPushButton::clicked, this, &MainWindow::onExport_PbClick);
    connect(ui->Export_PbSocket, &QPushButton::clicked, this, &MainWindow::onExport_PbSocketClick);

    connect(ndiImpl, &NDIModule::progressUpdate, this, &MainWindow::progressUpdate);

    // standby
    this->worker_cominit = new Worker_ComInit(ui->ComD_ListView, this->modelCom, this->comDetImpl);
    ui->ComD_PbReset->click();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete TimeStampImpl;
    delete camDetImpl;
    delete exptImpl;
    delete worker_cominit;
}

// ==================== ui交互部分 ============================================//
/*
更新进度条。
*/
void MainWindow::progressUpdate(int progress, const std::string &status){
    qDebug()<< progress <<" "<< status;
    progressLabel->setText(QString(status.c_str()));
    if (progress == 100){
        progressBar->setVisible(false);
    }
    else {
        progressBar->setValue(progress);
        progressBar->setVisible(true);
    }
}

/*
    * 更新画面
    * @param frame: 画面帧
*/
void MainWindow::updateFrame(const cv::Mat & frame){
    if (frame.empty()) return;
    cv::cvtColor(frame, cvframeDisp, cv::COLOR_BGR2RGB);
    QImage img = QImage((const unsigned char*)(cvframeDisp.data), cvframeDisp.cols, cvframeDisp.rows, cvframeDisp.cols*3 , QImage::Format_RGB888);
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
    if (this->ndiHandle[0]>0){
        auto m = poseMap.at(this->ndiHandle[0]);
        ui->NDI_LableOut0->setText(pose2str(m.get()));
    }
    if (this->ndiHandle[1]>0){
        auto m = poseMap.at(this->ndiHandle[1]);
        ui->NDI_LableOut1->setText(pose2str(m.get()));
    }
    if (this->ndiHandle[2]>0){
        auto m = poseMap.at(this->ndiHandle[2]);
        ui->NDI_LableOut2->setText(pose2str(m.get()));
    }
    if (this->ndiHandle[3]>0){
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
        this->opcvFrmImpl->getFrame(this->cvframe, this->clipROI);
    }
    if (this->ndiImpl->isOpened()){
        this->ndiData7.clear();
        this->ndiImpl->getPosition(this->ndiData7); //尺寸未知
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
        this->updatePose(this->ndiData7);
        details += "电磁定位输出类型：7\n";
    }

    // 数据导出
    if (this->onExportingLocal || this->onExportingSocket) {
        this->ui->NDI_LableType0->setText("导出类型：7");
        this->ui->NDI_LableType1->setText("导出类型：7");
        this->ui->NDI_LableType2->setText("导出类型：7");
        this->ui->NDI_LableType3->setText("导出类型：7");
        if (this->onExportingLocal){
                this->exptImpl->exportData<data_ptr7>(
                    exptIdx, cvframe,ndiHandle, ndiData7,
                    TimeStampImpl->getTimeStamp(), exptFolderPath);
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
    // ROI
    details += "clipROI:["+
               QString::number(this->clipROI.x)+","+ \
                QString::number(this->clipROI.y)+","+ \
                QString::number(this->clipROI.width)+","+ \
               QString::number(this->clipROI.height)+"]\n";
    //std::vector<double> fpsList;
    details += "FPS:" + this->TimeStampImpl->getFPS();
    this->exptIdx++;
    ui->OpcvF_LableDetail->setText(details);
}

// ====== 01 camera 相关交互 ======

void MainWindow::onCamD_PbConnectClick(){
    opcvFrmImpl->Close();
    ui->CamD_Label->setText("当前相机: 关闭");
    bool bOK = false;
    // 确保输入数字
    int camIndex = QInputDialog::getInt(this, "相机索引", "请输入相机索引", 0, 0, 10000, 1, &bOK);
    if (!bOK) return;
    // 激活相机
    if(camDetImpl->activateCam(camIndex) == false){
        QMessageBox::warning(this, "错误", "无法读取有效图片，请检查以下相机或文件路径是否有误:\n"+QString::fromStdString(camDetImpl->getActivateCam()));
        return;
    }
    ui->CamD_Label->setText("当前相机: "+QString::fromStdString(camDetImpl->getActivateCam()));
    // 打开相机, str转int
    opcvFrmImpl->Open(camIndex, this->frameHeight, this->frameWidth);

    if (this->onRunning == false){
        this->onRunning = true;
        this->timer->start(25); // Max FPS = 40
        this->ui->Export_PbRunPause->setText("暂停");
        this->ui->Export_Pb->setEnabled(true);
    }
}


// ======02 export 相关交互 ======
void MainWindow::onExport_PbRunPauseClick(){
    if (this->onRunning == true){
        this->onRunning = false;
        this->timer->stop();
        this->ui->Export_PbRunPause->setText("启动");
        this->onExportingLocal = false;
        this->ui->Export_Pb->setText("开始采集（到本地）");
        this->ui->Export_Pb->setEnabled(false);
    }
    else{
        this->onRunning = true;
        this->timer->start(25); // Max FPS = 40
        this->ui->Export_PbRunPause->setText("暂停");
        this->ui->Export_Pb->setEnabled(true);
    }
}

void MainWindow::onExport_PbClick() {
    if (this->onRunning){
        if (this->onExportingLocal == true) {
            this->onExportingLocal = false;
            this->ui->Export_Pb->setText("开始采集（到本地）");
        }
        else{
            this->onExportingLocal = true;
            this->ui->Export_Pb->setText("停止本地采集");
            //
            if(!this->onExportingSocket){  //确保都关闭才重置
                this->exptIdx = 0;
            }
            if (this->exptPath == "")
                this->exptFolderPath = "SampleOutput/" + this->TimeStampImpl->getTimeStamp();
            else{
                this->exptFolderPath = this->exptPath;
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
            
            this->exptImpl->SocketClose();
        }
        else{
            bool bOK = false;
            QString address = QInputDialog::getText(this, "地址", "请输入服务器开放的地址和端口（IP:Port）",
                QLineEdit::Normal, this->sock_address + ":" + QString::number(this->sock_port), &bOK);
            if (bOK)
            {
                this->progressUpdate(0, "解析IP地址和端口...");
                std::string ip = address.split(":").at(0).toStdString();
                int port = address.split(":").at(1).toInt();
                this->ui->Export_PbSocket->setText("建立服务器...");
                QApplication::processEvents();


                this->progressUpdate(70, "等待TCP连接中...");
                if(this->exptImpl->SocketInit(ip, port)){
                    this->onExportingSocket = true;
                    this->ui->Export_PbSocket->setText("关闭采集服务器");
                    this->progressUpdate(100, "连接成功。");
                    //
                    if(!this->onExportingLocal){  //确保都关闭才重置
                        this->exptIdx = 0;
                    }
                }
                else{
                    this->ui->Export_PbSocket->setText("启动采集服务器");
                    QMessageBox::warning(this, "错误", "无法启动服务器"+address);
                    this->exptImpl->SocketClose();
                    this->progressUpdate(100, "连接失败。");
                }
            }
        }
    }
    else {
        QMessageBox::warning(this, "错误", "请先点击\"启动\"以运行采集进程，再启动采集服务器！");
    }
}

// ====== 03 COM口 交互 ======

Worker_ComInit::Worker_ComInit(QListView *listView, QStandardItemModel *modelCom, COMPortDetection *comDetImpl) {
    this->listView = listView;
    this->modelCom = modelCom;
    this->comDetImpl = comDetImpl;
    onRun = false;
}

void Worker_ComInit::run(){
    if (onRun){
        return;
    }
    // 开始提示
    onRun = true;
    // 检查端口列表
    std::vector<std::string> comList = comDetImpl->getAvailablePorts();

    // 显示
    modelCom->clear();
    for(int i=0; i<comList.size(); i++){
        QStandardItem *item = new QStandardItem(QString::fromStdString(comList.at(i))); // 可能不会有内存泄漏危险，因为modelCom->clear()能帮助
        modelCom->appendRow(item);
    }
    // 加载到列表
    listView->setModel(modelCom);
    listView->setCurrentIndex(modelCom->index(0, 0));
    onRun = false;
    return;
}

void MainWindow::onComD_PbResetClick(){
    // 关闭播放
    if (this->onRunning == true){
        this->onRunning = false;
        this->timer->stop();
        this->ui->Export_PbRunPause->setText("启动");
    }
    // 清空


    // 关闭端口
    this->progressUpdate(25, "正在终止数据采集进程...");
    this->ndiImpl->StopTracking();
    this->progressUpdate(50, "正在关闭现有的串行连接...");
    this->ndiImpl->Close();
    ui->ComD_Label->setText("当前端口: 关闭");

    // 检查端口列表
    this->progressUpdate(75, "正在重新扫描所有可用串行端口...");
    modelCom->clear();
    QStandardItem *item = new QStandardItem("正在检查串行端口...");
    modelCom->appendRow(item);
    ui->ComD_ListView->setModel(modelCom);
    QApplication::processEvents();
    // 加载到列表
    this->worker_cominit->start();

    this->progressUpdate(100, "串行端口重置完毕。");
    return;
}

void MainWindow::onCamD_PbChangeRClick(){
    //若已指定则询问是否更改
    if (this->clipROI.width > 0){
        int ret = QMessageBox::question(this, "更改ROI", "警告：已经有来自参数的ROI可用。强行更改ROI可能导致尺度因子标定失效。\n是否更改ROI？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::No) return;
    }
    if (opcvFrmImpl->isOpened()){
        bool bOK = false;
        int height = QInputDialog::getInt(this, "高度", "请输入视频流高度", 1080, 0, 10000, 1, &bOK);
        if (!bOK) return;
        int width = QInputDialog::getInt(this, "宽度", "请输入视频流宽度", 1920, 0, 10000, 1, &bOK);
        if (!bOK) return;
        opcvFrmImpl->changeSize(height, width);
    }
    else{
        QMessageBox::warning(this, "错误", "请先连接相机!");
    }
    return;
}

void MainWindow::onComD_PbConnectClick(){
    // 关闭电磁定位
    if (this->ndiImpl->isOpened()){
        this->ndiImpl->Close();
    }
    // 读取
    std::string comName = ui->ComD_ListView->currentIndex().data().toString().toStdString();
    this->comDetImpl->activateCOM(comName);

    try {
        ndiImpl->Initialize(true, this->comDetImpl->getActivateCOM());
    }
    catch (std::runtime_error &e) {
        this->progressUpdate(100, "NDI Aurora设备初始化失败。");
        QMessageBox::warning(this, "初始化时发生的错误", QString::fromStdString(e.what()));
        return;
    }

    try {
        ndiImpl->Open();
        this->ndiHandle = this->ndiImpl->getHandlers();
        assert(ndiHandle.size() == 4);
    }
    catch (std::runtime_error &e) {
        QMessageBox::warning(this, "启动跟踪时发生的错误", QString::fromStdString(e.what()));
        return;
    }

    // ui更新
    if (this->ndiHandle[0]>0) {
        this->ui->NDI_LableState0->setText("状态：connected");
    }
    else {
        this->ui->NDI_LableState0->setText("状态：disconnected");
    }
    if (this->ndiHandle[1]>0) {
        this->ui->NDI_LableState1->setText("状态：connected");
    }
    else {
        this->ui->NDI_LableState1->setText("状态：disconnected");
    }
    if (this->ndiHandle[2]>0) {
        this->ui->NDI_LableState2->setText("状态：connected");
    }
    else {
        this->ui->NDI_LableState2->setText("状态：disconnected");
    }
    if (this->ndiHandle[3]>0) {
        this->ui->NDI_LableState3->setText("状态：connected");
    }
    else {
        this->ui->NDI_LableState3->setText("状态：disconnected");
    }

    // 显示
    if (this->onRunning == false){
        this->onRunning = true;
        this->timer->start(25); // Max FPS = 40
        this->ui->Export_PbRunPause->setText("暂停");
        this->ui->Export_Pb->setEnabled(true);
    }
}

void MainWindow::onPbShowSaveClick(){
    // 打开保存文件夹
    QDesktopServices::openUrl(QUrl::fromLocalFile(this->exptFolderPath));
    return;
}


