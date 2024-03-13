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

    // connect
    connect(timer, &QTimer::timeout, this, &MainWindow::onTime);
    connect(ui->DelyaC_Pb, &QPushButton::clicked, this, &MainWindow::onDelyaC_PbClick);
    connect(ui->CamD_PbReset, &QPushButton::clicked, this, &MainWindow::onCamD_PbResetClick);
    connect(ui->CamD_PbConnect, &QPushButton::clicked, this, &MainWindow::onCamD_PbConnectClick);
    connect(ui->Export_Pb_runPause, &QPushButton::clicked, this, &MainWindow::onExport_PbRunPauseClick);
    // standby
    ui->CamD_PbReset->click();
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
void MainWindow::updateFrame(cv::Mat frame){
    QImage img = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    QPixmap img2 = QPixmap::fromImage(img).scaled(ui->OpcvF_LableFrame->size(), Qt::KeepAspectRatio,Qt::SmoothTransformation);
    //QPixmap img2 = QPixmap::fromImage(img);
    ui->OpcvF_LableFrame->setPixmap(img2);
}

/*
    * 主循环更新函数。包括：
    * 1. 时间戳更新
    * 2. 画面更新
    * 3. 位姿更新
    * 4. 数据导出
    * 5. 信息显示
*/
void MainWindow::onTime(){
    // 时间戳更新
    ui->TimeS_Label->setText("当前时间戳: \t"+this->TimeStampImpl->getTimeStamp());
    // 更新画面
    if(this->opcvFrmImpl->isOpened())
        this->updateFrame(this->opcvFrmImpl->getFrame());
    // 更新位姿
    // 数据导出
    // 信息显示
    std::string details = opcvFrmImpl->getInfo();
    ui->OpcvF_LableDetail->setText(QString::fromStdString(details));
}

void MainWindow::onDelyaC_PbClick() {
    this->DelayClibImpl->doDelayCalib();
    int frameDelayMs = this->DelayClibImpl->getFrameDelayMs();
    this->ui->OpcvF_LableDetail->setText(
        "延迟："+QString::number(frameDelayMs)+" ms"
        );
}

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
