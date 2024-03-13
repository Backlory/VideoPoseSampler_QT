#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 实例化 DelayClib
    DelayClib = new DelayCalibration(this);
    // 实例化 TimeStamp
    TimeStampImpl = new TimeStamp();


    // 连接
    connect(ui->DelyaC_Pb, &QPushButton::clicked, this, &MainWindow::onButtonClick);

    // 连接时间戳，修改 timer->start(*) 内容以达到修改刷新频率的功能
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::showTimeStamp);
    timer->start(33);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete DelayClib;
    delete TimeStampImpl;

}

void MainWindow::onButtonClick() {
    this->DelayClib->doDelayCalib();
    int frameDelayMs = this->DelayClib->getFrameDelayMs();
    this->ui->OpcvF_LableDetail->setText(
        "延迟："+QString::number(frameDelayMs)+" ms"
        );
}

void MainWindow::showTimeStamp(){
    //使时间戳标签显示当前的时间戳
    ui->TimeS_Label->setText("当前时间戳: \t"+this->TimeStampImpl->getTimeStamp());
}

