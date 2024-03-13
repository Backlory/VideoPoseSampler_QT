#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QDateTime>

class TimeStamp {
public:
    TimeStamp(){};
    QString getTimeStamp();
private:
    qint64 currentTimestamp;
};

#endif // TIMESTAMP_H
