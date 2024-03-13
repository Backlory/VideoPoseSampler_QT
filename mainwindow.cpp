#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // instantiation
    DelayClib = new DelayCalibration(this);
    TimeStampImpl = new TimeStamp();
    timer = new QTimer(this);

    // connect
    connect(ui->DelyaC_Pb, &QPushButton::clicked, this, &MainWindow::onDelyaC_PbClick);
    connect(timer, &QTimer::timeout, this, &MainWindow::showTimeStamp);

    // standby
    timer->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete DelayClib;
    delete TimeStampImpl;
}

// ==================== ui交互部分 ============================================//

void MainWindow::onDelyaC_PbClick() {
    this->DelayClib->doDelayCalib();
    int frameDelayMs = this->DelayClib->getFrameDelayMs();
    this->ui->OpcvF_LableDetail->setText(
        "延迟："+QString::number(frameDelayMs)+" ms"
        );
}

void MainWindow::showTimeStamp(){
    ui->TimeS_Label->setText("当前时间戳: \t"+this->TimeStampImpl->getTimeStamp());
}

