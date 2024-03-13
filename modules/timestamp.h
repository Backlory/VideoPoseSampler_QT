#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QDateTime>

class TimeStamp {
public:
    TimeStamp(){};
    QString getTimeStamp();
private:
    qint64 currentTimestamp;
    QDateTime currentDateTime;
};

#endif // TIMESTAMP_H
