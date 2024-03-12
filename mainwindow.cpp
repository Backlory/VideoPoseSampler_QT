#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 实例化
    DelayClib = new DelayCalibration(this);
    // 连接

    connect(ui->DelyaC_Pb, &QPushButton::clicked, this, &MainWindow::onButtonClick);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete DelayClib;
}

void MainWindow::onButtonClick() {
    this->DelayClib->doDelayCalib();
    int frameDelayMs = this->DelayClib->getFrameDelayMs();
    this->ui->OpcvF_LableDetail->setText(
        "延迟："+QString::number(frameDelayMs)+" ms"
        );
}
