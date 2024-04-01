#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QDateTime>
#include <vector>
#include "highresolutiontime.h"
class TimeStamp {
public:
    TimeStamp(){};
    QString getTimeStamp();
    QString getFPS();
private:
    qint64 currentTimestamp;
    QDateTime currentDateTime;
    std::vector<double> fpsList;
};

#endif // TIMESTAMP_H
