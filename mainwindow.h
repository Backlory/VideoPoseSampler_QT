#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <qstring.h>
#include <string.h>
#include <QTimer>
#include <QDateTime>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMessageBox>
#include <QImage>
#include <QInputDialog>
#include <QDesktopServices>
#include <QThread>
#include <QListView>
#include <QProgressBar>
#include <QLabel>
#include <QDebug>

#include "Sophus/so3.hpp"

#include "modules/cameradetection.h"
#include "modules/comportdetection.h"
#include "modules/export.h"
#include "modules/ndimodule.h"
#include "modules/export.h"
#include "modules/opencvframe.h"
#include "modules/timestamp.h"

using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class Worker_ComInit: public QThread
{
public:
    Worker_ComInit(QListView *listView, QStandardItemModel *modelCom, COMPortDetection *comDetImpl);
    void run() override;

private:
    QListView *listView;
    QStandardItemModel *modelCom;
    COMPortDetection *comDetImpl;
    bool onRun;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, QString address = "", int port = 0, QString saveDir = "", QStringList roiValues= {}, QStringList sizeValues= {});
    ~MainWindow();

private:
    Ui::MainWindow *ui;
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
    cv::Rect clipROI;
    int frameHeight, frameWidth;
    std::vector<int> ndiHandle;  //idx->port name, size == 4
    std::map<int, data_ptr7> ndiData7;  //port name -> port value
    void updateFrame(const cv::Mat &);
    void updatePose(const std::map<int, data_ptr7>);
    // 进度条
    QLabel *progressLabel;
    QProgressBar *progressBar;
    // ndi显示
    bool ndiActivated0 = false;
    bool ndiActivated1 = false;
    bool ndiActivated2 = false;
    bool ndiActivated3 = false;
    std::map<int, std::shared_ptr<Sophus::Vector6d>> ndiOut6;
    std::map<int, std::shared_ptr<Eigen::Matrix4d>> ndiOut4;
    //数据导出
    Export * exptImpl;
    QString exptPath;
    QString sock_address;
    int sock_port;
    int exptIdx = 0;
    std::vector<double> fpsList;
    QString exptFolderPath;
    //多线程
    Worker_ComInit *worker_cominit;

private slots:
    void progressUpdate(int progress, const std::string &status);

public slots:
    void onTime();
    void onCamD_PbConnectClick();
    void onCamD_PbChangeRClick();
    void onComD_PbConnectClick();
    void onComD_PbResetClick();
    void onExport_PbRunPauseClick();
    void onExport_PbClick();
    void onExport_PbSocketClick();
    void onPbShowSaveClick();
};



#endif // MAINWINDOW_H
