/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-03-13 23:02:09
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-09-14 11:10:36
 * @Description: 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
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
// 这与
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
