#include "delaycalibration.h"

DelayCalibration::DelayCalibration(QWidget * parents)
{
    this->frameDelayMs = 0;
    this->parents = parents;
}


void DelayCalibration::doDelayCalib(){
    bool bOK = false;
    int delayFrameNum = QInputDialog::getInt(this->parents, "延迟标定", "请输入图像延迟的毫秒数",
                                                 200, 0, 9999, 5, &bOK);
    if (bOK && delayFrameNum>=0){
        this->frameDelayMs = delayFrameNum;
    }
}


int DelayCalibration::getFrameDelayMs() const {
    return this->frameDelayMs;
}
