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

using data_ptr4 = std::shared_ptr<Eigen::Matrix4d>;
using data_ptr6 = std::shared_ptr<Sophus::Vector6d>;
using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

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
    QStandardItemModel * modelCom;
    COMPortDetection *comDetImpl;
    // 数据采集
    OpenCVFrame * opcvFrmImpl;
    NDIModule * ndiImpl;
    bool onRunning = false;
    bool onExportingLocal = false;
    bool onExportingSocket = false;
    cv::Mat cvframe;
    cv::Mat cvframeDisp;
    std::vector<int> ndiHandle;  //idx->port name, size == 4
    std::map<int, data_ptr4> ndiData4;  //port name -> port value
    std::map<int, data_ptr6> ndiData6;
    std::map<int, data_ptr7> ndiData7;
    void updateFrame(const cv::Mat &);
    void updatePose(const std::map<int, data_ptr7>);
    void updatePose(const std::map<int, data_ptr6>);
    void updatePose(const std::map<int, data_ptr4>);
    // ndi显示
    int ndiOutputType = 6;
    bool ndiActivated0 = false;
    bool ndiActivated1 = false;
    bool ndiActivated2 = false;
    bool ndiActivated3 = false;
    std::map<int, std::shared_ptr<Sophus::Vector6d>> ndiOut6;
    std::map<int, std::shared_ptr<Eigen::Matrix4d>> ndiOut4;
    //数据导出
    Export * exptImpl;
    int exptIdx = 0;
    QString exptFolderPath;



public slots:
    void onTime();
    void onDelyaC_PbClick();
    void onCamD_PbResetClick();
    void onCamD_PbConnectClick();
    void onComD_PbConnectClick();
    void onComD_PbResetClick();
    void onExport_PbRunPauseClick();
    void onExport_PbClick();
    void onExport_PbSocketClick();
    void onNDI_Cb0Click();  // 复选框
    void onNDI_Cb1Click();
    void onNDI_Cb2Click();
    void onNDI_Cb3Click();
};

#endif // MAINWINDOW_H
