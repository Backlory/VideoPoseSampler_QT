#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include "modules/cameradetection.h"
#include "modules/comportdetection.h"
#include "modules/delaycalibration.h"
#include "modules/export.h"
#include "modules/jsonloading.h"
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
    DelayCalibration *DelayClib;
public slots:
    void onDelyaC_PbClick();
};

#endif // MAINWINDOW_H
