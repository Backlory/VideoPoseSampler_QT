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

