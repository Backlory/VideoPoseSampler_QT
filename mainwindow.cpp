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
    connect(ui->DelyaC_Pb, &QPushButton::clicked, this, &MainWindow::onDelyaC_PbClick);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete DelayClib;
}

/* =========================== ui 交互更新函数 =================================================== */
void MainWindow::onDelyaC_PbClick() {
    this->DelayClib->doDelayCalib();
    int frameDelayMs = this->DelayClib->getFrameDelayMs();
    this->ui->OpcvF_LableDetail->setText(
        "延迟："+QString::number(frameDelayMs)+" ms"
        );
}
