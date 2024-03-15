#ifndef EXPORT_H
#define EXPORT_H

#include <QString>
#include <QDir>

#include "../3rdparty/Aurora/APIStructures.h"
#include "opencv2/opencv.hpp"
#include "Sophus/so3.hpp"
#include "boost/thread.hpp"



class Export{

    using data_ptr4 = std::shared_ptr<Eigen::Matrix4d>;
    using data_ptr6 = std::shared_ptr<Sophus::Vector6d>;
    using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

public:
    Export();
    ~Export();

    void createFolder(const QString) const;

    static void d7to4(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr4> &d4);
    static void d7to6(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr6> &d6);

    template <typename T>
    void exportData(const int, const cv::Mat, const std::map<int, T>, QString, QString) const;

private:
    template <typename T>
    void exportThread(const int frameIndex, const cv::Mat frame, const std::map<int, T> ndiData,
                  const QString timeStamp, const QString savePath) const;

    void saveFrame(const QString, const cv::Mat, const QString ) const;
    template <typename T>
    void savePoseWithTimeStamp(const QString frameIndexStr, const std::map<int, T> ndiData,
                            const QString path, const QString savePath) const ;
};
#endif // EXPORT_H
