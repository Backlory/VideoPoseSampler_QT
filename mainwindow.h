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
    void updateFrame(cv::Mat);
    //void updatePose(std::vector<Q>);

private:
    Ui::MainWindow *ui;
    DelayCalibration *DelayClibImpl;
    //
    QTimer * timer;
    TimeStamp *TimeStampImpl;
    //
    QStandardItemModel * modelCam;
    CameraDetection *camDetImpl;
    //
    OpenCVFrame * opcvFrmImpl;
    bool onRunning = false;
    //



public slots:
    void onTime();
    void onDelyaC_PbClick();
    void onCamD_PbResetClick();
    void onCamD_PbConnectClick();
    void onExport_PbRunPauseClick();
    //todo
};

#endif // MAINWINDOW_H
