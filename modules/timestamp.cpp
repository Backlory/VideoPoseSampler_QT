#include "timestamp.h"
/*
    DO:     get the time stamp from local env and percision to millisecond
    param:  null
    return: string (current time stamp percision to millisecond)
*/
QString TimeStamp::getTimeStamp()
{
    // use QDataTime get the timestamp which percision to millisecond,the return is a *long long int* shows the millisecond time from 1970
    qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch();
    // chang the element type from *long long int* to *q date time*
    QDateTime currentDateTime = QDateTime::fromMSecsSinceEpoch(currentTimestamp);
    // from time to String
    QString timestampStr = currentDateTime.toString("yyyy-MM-dd hh:mm:ss:zzz");

    return timestampStr;
}

