#ifndef DELAYCALIBRATION_H
#define DELAYCALIBRATION_H

#include <QInputDialog>

class DelayCalibration {
public:
    DelayCalibration(QWidget *);
    ~DelayCalibration(){};
    //
    void doDelayCalib();
    int getFrameDelayMs();
    //
private:
    int frameDelayMs = 0;
    QWidget * parents;
};

#endif // DELAYCALIBRATION_H
