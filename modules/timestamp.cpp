#include "timestamp.h"
/*
    DO:     get the time stamp from local env and percision to millisecond
    param:  null
    return: string (current time stamp percision to millisecond)
*/
QString TimeStamp::getTimeStamp()
{
    this->currentTimestamp = QDateTime::currentMSecsSinceEpoch();
    this->currentDateTime = QDateTime::fromMSecsSinceEpoch(this->currentTimestamp);
    return this->currentDateTime.toString("yyyyMMdd_hhmmss_zzz");
}


// 获取当前FPS。
// 需要多次调用才能计算出FPS
QString TimeStamp::getFPS()
{
    fpsList.push_back(HSTime::wall_time());
    if (fpsList.size() < 30) {
        return "...";
    }
    else {
        fpsList.erase(fpsList.begin());
        double fps = 30.0 / (fpsList.back() - fpsList.front());
        return QString::number(fps, 'f', 2);
    }
}
