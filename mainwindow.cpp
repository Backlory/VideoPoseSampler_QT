#include "mainwindow.h"
#include "ui_mainwindow.h"

#define _MAJOR_VERSION_ "1"
#define _MINOR_VERSION_ "3"
#define _PATCH_VERSION_ "6"


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

    localDwPPlayerImpl = new LocalDwPPlayer();

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

    connect(ui->LocalDwP_PbLoadDwP, &QPushButton::clicked, this, &MainWindow::onLocalDwP_PbLoadDwPClick);
    connect(ui->LocalDwP_PbUnloadDwP, &QPushButton::clicked, this, &MainWindow::onLocalDwP_PbUnloadDwPClick);
    connect(ui->LocalDwP_HsTimeline, &QSlider::valueChanged, this, &MainWindow::onLocalDwP_HsTimelineValueChanged);
    connect(ui->LocalDwP_CbRecurrent, &QCheckBox::clicked, this, &MainWindow::onLocalDwP_CbRecurrentClick);
    connect(ui->LocalDwP_pBSetFPS, &QPushButton::clicked, this, &MainWindow::onLocalDwP_pBSetFPSClick);
    connect(ui->LocalDwP_PbJumpTo, &QPushButton::clicked, this, &MainWindow::onLocalDwP_PbJumpToClick);
    connect(ui->LocalDwP_PbPlayStop, &QPushButton::clicked, this, &MainWindow::onLocalDwP_PbPlayStopClick);



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
progress 为 100时，隐藏进度条。
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

QString pose2str(QuatTransformationStruct * m, const double mErr){
    QString str;
    str += "x:" + QString::number(m->translation.x) + "\n";
    str += "y:" + QString::number(m->translation.y) + "\n";
    str += "z:" + QString::number(m->translation.z) + "\n";
    str += "q0:" + QString::number(m->rotation.q0) + "\n";
    str += "qx:" + QString::number(m->rotation.qx) + "\n";
    str += "qy:" + QString::number(m->rotation.qy) + "\n";
    str += "qz:" + QString::number(m->rotation.qz) + "\n";
    str += "Error index:" + QString::number(mErr);
    return str;
}

void MainWindow::updatePose(const std::map<int, data_ptr7> poseMap, std::map<int, double> poseMapErr){
    //qDebug() << 0 << this->ndiHandle;
    if (this->ndiHandle[0]>0){
        auto m = poseMap.at(this->ndiHandle[0]); //data_ptr7, 需要get()
        auto mErr = poseMapErr.at(this->ndiHandle[0]); //double
        ui->NDI_LableOut0->setText(pose2str(m.get(), mErr));
    }
    if (this->ndiHandle[1]>0){
        auto m = poseMap.at(this->ndiHandle[1]);
        auto mErr = poseMapErr.at(this->ndiHandle[1]);
        ui->NDI_LableOut1->setText(pose2str(m.get(), mErr));
    }
    if (this->ndiHandle[2]>0){
        auto m = poseMap.at(this->ndiHandle[2]);
        auto mErr = poseMapErr.at(this->ndiHandle[2]);
        ui->NDI_LableOut2->setText(pose2str(m.get(), mErr));
    }
    if (this->ndiHandle[3]>0){
        auto m = poseMap.at(this->ndiHandle[3]);
        auto mErr = poseMapErr.at(this->ndiHandle[3]);
        ui->NDI_LableOut3->setText("NDI0: \n" + pose2str(m.get(), mErr));
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

    if(this->localDwPPlayerImpl->isReadyToGrab()){
        this->localDwPPlayerImpl->getOneFrame(this->cvframe, this->ndiHandle, this->ndiData7, this->ndiData7Err);
        this->ui->LocalDwP_HsTimeline->setValue(this->localDwPPlayerImpl->getNowIdx());
        this->updateFrame(this->cvframe);
        this->updatePose(this->ndiData7, this->ndiData7Err);
    }
    else {
        // 同步触发
        if (this->opcvFrmImpl->isOpened()) {
            this->opcvFrmImpl->getFrame(this->cvframe, this->clipROI);
        }
        if (this->ndiImpl->isOpened()){
            this->ndiData7.clear();
            this->ndiImpl->getPosition(this->ndiData7, this->ndiData7Err); //尺寸未知
        }
    }

    QString details;
    details += "导出序号：" +  QString::number(this->exptIdx) + "\n";
    details += "导出位置：" + this->exptFolderPath + "\n";
    // 更新画面
    if (this->opcvFrmImpl->isOpened()) {
        this->updateFrame(this->cvframe);
        details += QString::fromStdString(opcvFrmImpl->getInfo());
    }

    // 更新位姿z
    if (this->ndiImpl->isOpened()){
        this->updatePose(this->ndiData7, this->ndiData7Err);
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
            int m = this->exptImpl->exportSocketData(exptIdx, cvframe,
                                                      ndiHandle, ndiData7,
                                                      TimeStampImpl->getTimeStamp());
            if(m >= 3) {
                this->progressUpdate(100, "failed code ="+ std::to_string(m));
                //QMessage询问是否断开连接
                //int ret = QMessageBox::question(this, "错误", "客户端连接中断，错误代码="+QString::number(m)+"\n是否要断开TCP连接？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                //if (ret == QMessageBox::Yes) {
                //    this->ui->Export_PbSocket->click();
                //}
            }
        }
        details += "正在本地导出:" + QString(this->onExportingLocal?"true":"false") + "\n";
        details += "正在端口导出:" + QString(this->onExportingSocket?"true":"false") + "\n";
    }
    // ROI
    if(this->localDwPPlayerImpl->isReadyToGrab()){
        details += "clipROI:[do not use]\n";
    }
    else {
        details += "clipROI:["+
                   QString::number(this->clipROI.x)+","+ \
                    QString::number(this->clipROI.y)+","+ \
                    QString::number(this->clipROI.width)+","+ \
                   QString::number(this->clipROI.height)+"]\n";
    }
    //std::vector<double> fpsList;
    details += "FPS:" + this->TimeStampImpl->getFPS();
    this->exptIdx++;
    ui->OpcvF_LableDetail->setText(details);
}

// ====== 01a camera 相关交互 ======

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

// ====== 01b COM口 交互 ======

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
    this->progressUpdate(25, "Terminating data collection process...");
    this->ndiImpl->StopTracking();
    this->progressUpdate(50, "Closing existing serial connection...");
    this->ndiImpl->Close();
    ui->ComD_Label->setText("当前端口: 关闭");

    // 检查端口列表
    this->progressUpdate(75, "Rescan all available serial ports...");
    modelCom->clear();
    QStandardItem *item = new QStandardItem("正在检查串行端口...");
    modelCom->appendRow(item);
    ui->ComD_ListView->setModel(modelCom);
    QApplication::processEvents();
    // 加载到列表
    this->worker_cominit->start();

    this->progressUpdate(100, "Serial port reset complete.");
    return;
}

void MainWindow::onCamD_PbChangeRClick(){
    // 如果正在运行，点击Export_PbRunPause
    if (this->onRunning == true){
        this->ui->Export_PbRunPause->click();
    }
    //若已指定则询问是否更改
    if (this->clipROI.width > 0){
        int ret = QMessageBox::question(this, "更改ROI", "警告：已经有来自参数的ROI可用。强行更改ROI可能导致尺度因子标定失效。\n是否更改ROI？", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::No) return;
    }
    if (opcvFrmImpl->isOpened()){
        int height, width;
        auto dialog = selectResolution(this);
        dialog.setModal(true);
        if (dialog.exec() == QDialog::Accepted){
            dialog.getResolution(width, height);
            opcvFrmImpl->changeSize(height, width);
        }
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
    if (comName == "" || comName == "正在检查串行端口..."){
        bool bOK = false;
        int comIndex = QInputDialog::getInt(this, "串口索引", "请输入串口号(数字)", 0, 4, 100, 1, &bOK);
        if (!bOK){
            return;
        }
        comName = "COM" + std::to_string(comIndex);
        this->progressUpdate(100, "manually input com port = "+comName);
        this->comDetImpl->activateCOM(comName);
    }
    else {
        this->comDetImpl->activateCOM(comName);
    }

    try {
        ndiImpl->Initialize(true, this->comDetImpl->getActivateCOM());
    }
    catch (std::runtime_error &e) {
        this->progressUpdate(100, "NDI Aurora device init failed.");
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

// ============== 02 本地数据集相关交互 ==============

void MainWindow::onLocalDwP_PbLoadDwPClick(){
    if (this->onRunning == true) {
        this->ui->Export_PbRunPause->click();
    }
    // 打开本地文件夹
    QString path = QFileDialog::getExistingDirectory(this, "选择数据集文件夹", "");
    if (path.isEmpty()){
        return;
    }
    else {
        bool ret = this->localDwPPlayerImpl->loadDwP(path.toStdString());
        if (ret) {
            this->ui->LocalDwP_PbLoadDwP->setEnabled(false);
            this->ui->LocalDwP_PbUnloadDwP->setEnabled(true);
            this->ui->LocalDwP_HsTimeline->setEnabled(true);
            this->ui->LocalDwP_HsTimeline->setMinimum(0);
            this->ui->LocalDwP_HsTimeline->setMaximum(this->localDwPPlayerImpl->getLength());
            this->ui->LocalDwP_HsTimeline->setValue(0);
            this->ui->LocalDwP_CbRecurrent->setEnabled(true);
            this->ui->LocalDwP_CbRecurrent->setChecked(false);
            //this->ui->LocalDwP_pBSetFPS->setEnabled(true);
            this->ui->LocalDwP_PbJumpTo->setEnabled(true);
            this->ui->LocalDwP_PbPlayStop->setEnabled(true);
            this->ui->LocalDwP_PbPlayStop->setText("播放");
            this->ui->LocalDwP_LbFrameIdx->setText("帧编号："
                + QString::number(this->localDwPPlayerImpl->getNowIdx()) 
                + "/" 
                + QString::number(this->localDwPPlayerImpl->getLength())); 
        }
        else {
            QMessageBox::warning(this, "错误", "无法加载数据集，请检查文件夹是否存在或者文件格式是否正确。需要满足：\n\t1、frame、PoseWithTimeStamp文件夹；\n\t2、图像和位姿索引连续且一致。\n");
        }
    }
}
void MainWindow::onLocalDwP_PbUnloadDwPClick(){
    if (this->onRunning == true) {
        this->ui->Export_PbRunPause->click();
    }
    if (this->localDwPPlayerImpl->unloadDwP()){
        this->ui->LocalDwP_PbLoadDwP->setEnabled(true);
        this->ui->LocalDwP_PbUnloadDwP->setEnabled(false);
        this->ui->LocalDwP_HsTimeline->setEnabled(false);
        this->ui->LocalDwP_CbRecurrent->setEnabled(false);
        //this->ui->LocalDwP_pBSetFPS->setEnabled(false);
        this->ui->LocalDwP_PbJumpTo->setEnabled(false);
        this->ui->LocalDwP_PbPlayStop->setEnabled(false);
        this->ui->LocalDwP_PbPlayStop->setText("播放");
        this->ui->LocalDwP_LbFrameIdx->setText("帧编号：0/0");
    }
    else {
        QMessageBox::warning(this, "错误", "无法卸载数据集，请检查是否已经加载数据集。");
    }
}

void MainWindow::onLocalDwP_HsTimelineValueChanged(){
    if (this->localDwPPlayerImpl->isReadyToGrab()){
        this->localDwPPlayerImpl->jumpTo(this->ui->LocalDwP_HsTimeline->value());
        this->ui->LocalDwP_LbFrameIdx->setText("帧编号："
            + QString::number(this->localDwPPlayerImpl->getNowIdx())
            + "/"
            + QString::number(this->localDwPPlayerImpl->getLength()));
    }
}

void MainWindow::onLocalDwP_CbRecurrentClick(){
    if (this->ui->LocalDwP_CbRecurrent->isChecked()){
        this->localDwPPlayerImpl->setRecurrence(true);
    }
    else {
        this->localDwPPlayerImpl->setRecurrence(false);
    }
}
void MainWindow::onLocalDwP_pBSetFPSClick(){
    // 输入数字，小数，默认30.0，最大100.0，最小1.0
    bool bOK = false;
    double fps = QInputDialog::getDouble(this, "设置FPS", "请输入FPS", 30.0, 1.0, 100.0, 1, &bOK);
    if (bOK){
        this->localDwPPlayerImpl->setFPS(fps);
        this->ui->LocalDwP_LbFPS->setText("FPS: " + QString::number(fps));
    }
}
void MainWindow::onLocalDwP_PbJumpToClick(){
    // 输入数字，整数，默认0，最大frameCount，最小0
    bool bOK = false;
    int frameIdx = QInputDialog::getInt(this, "跳转到帧", "请输入帧数", 0, 0, this->localDwPPlayerImpl->getLength(), 1, &bOK);
    if (bOK){
        this->ui->LocalDwP_HsTimeline->setValue(frameIdx);
    }
    return;
}
void MainWindow::onLocalDwP_PbPlayStopClick(){
    if (this->localDwPPlayerImpl->isReadyToGrab()){
        this->localDwPPlayerImpl->flipFlag();
        if (this->localDwPPlayerImpl->isPlaying()) {
            this->ui->LocalDwP_PbPlayStop->setText("暂停");
        }
        else {
            this->ui->LocalDwP_PbPlayStop->setText("播放");
        }
    }
    else {
        QMessageBox::warning(this, "错误", "请先加载数据集。");
    }
}


// ======03 export 相关交互 ======
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
                this->progressUpdate(0, "Resolve IP address and port...");
                std::string ip = address.split(":").at(0).toStdString();
                int port = address.split(":").at(1).toInt();
                this->ui->Export_PbSocket->setText("建立服务器...");
                QApplication::processEvents();

                this->progressUpdate(70, "Waitting for TCP connection...");
                if(this->exptImpl->SocketInit(ip, port)){
                    this->onExportingSocket = true;
                    this->ui->Export_PbSocket->setText("关闭采集服务器");
                    this->progressUpdate(100, "connected.");
                    //
                    if(!this->onExportingLocal){  //确保都关闭才重置
                        this->exptIdx = 0;
                    }
                }
                else{
                    this->ui->Export_PbSocket->setText("启动采集服务器");
                    QMessageBox::warning(this, "错误", "无法启动服务器"+address);
                    this->exptImpl->SocketClose();
                    this->progressUpdate(100, "connection failed.");
                }
            }
        }
    }
    else {
        QMessageBox::warning(this, "错误", "请先点击\"启动\"以运行采集进程，再启动采集服务器！");
    }
}

void MainWindow::onPbShowSaveClick(){
    // 打开保存文件夹
    QDesktopServices::openUrl(QUrl::fromLocalFile(this->exptFolderPath));
    return;
}


