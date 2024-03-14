#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <qstring.h>
#include <QTimer>
#include <QDateTime>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMessageBox>
#include <QImage>

#include "Sophus/so3.hpp"

#include "modules/cameradetection.h"
#include "modules/comportdetection.h"
#include "modules/delaycalibration.h"
#include "modules/export.h"
#include "modules/Configloading.h"
#include "modules/ndimodule.h"
#include "modules/export.h"
#include "modules/opencvframe.h"
#include "modules/timestamp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DelayCalibration *DelayClibImpl;
    //
    QTimer * timer;
    TimeStamp *TimeStampImpl;
    //
    QStandardItemModel * modelCam;
    CameraDetection *camDetImpl;
    // 数据采集
    OpenCVFrame * opcvFrmImpl;
    NDIModule * ndiImpl;
    bool onRunning = false;
    cv::Mat cvframe;
    std::vector<int> ndiHandle;
    std::map<int, data_ptr> ndiData;
    void updateFrame(const cv::Mat);
    void updatePose(const std::map<int, data_ptr>);
    // ndi显示
    int ndiOutputType0 = 6;
    int ndiOutputType1 = 6;
    int ndiOutputType2 = 6;
    int ndiOutputType3 = 6;
    bool ndiActivated0 = false;
    bool ndiActivated1 = false;
    bool ndiActivated2 = false;
    bool ndiActivated3 = false;
    std::map<int, std::shared_ptr<Sophus::Vector6d>> ndiOut6;
    std::map<int, std::shared_ptr<Eigen::Matrix4d>> ndiOut4;



public slots:
    void onTime();
    void onDelyaC_PbClick();
    void onCamD_PbResetClick();
    void onCamD_PbConnectClick();
    void onExport_PbRunPauseClick();
    void onComD_PbConnectClick();
    void onComD_PbResetClick();
    //
    void onNDI_Rb0_6Click(); // 选择6维
    void onNDI_Rb1_6Click();
    void onNDI_Rb2_6Click();
    void onNDI_Rb3_6Click();
    void onNDI_Rb0_4Click(); // 选择4阶
    void onNDI_Rb1_4Click();
    void onNDI_Rb2_4Click();
    void onNDI_Rb3_4Click();
    void onNDI_Cb0Click();  // 复选框
    void onNDI_Cb1Click();
    void onNDI_Cb2Click();
    void onNDI_Cb3Click();
};

#endif // MAINWINDOW_H
