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
    void exportData(const int, const cv::Mat,
                    const std::vector<int>, const std::map<int, T>,
                    QString, QString) const;

    template <typename T>
    void ndiData2QString(const T data, QString &) const;

private:
    void saveFrame(const QString, const cv::Mat, const QString ) const;
    template <typename T>
    void exportThread(const int frameIndex, const cv::Mat frame,
                      const std::vector<int>, const std::map<int, T> ndiData,
                  const QString timeStamp, const QString savePath) const;

    template <typename T>
    void savePoseWithTimeStamp(const std::vector<int> ndiHandle, const std::map<int, T> ndiData,
                            const QString timeStamp, const QString filePath) const ;
    //
};

template <typename T>
void Export::exportData(const int frameIndex, const cv::Mat frame,
                        const std::vector<int> ndiHandle, const std::map<int, T> ndiData, \
                        QString timeStamp, QString savePath) const {
    assert(ndiHandle.size() == 4);
    exportThread<T>(frameIndex, frame, ndiHandle, ndiData, timeStamp, savePath);
    boost::thread td(&Export::exportThread<T>, this, frameIndex, frame, ndiHandle, ndiData, timeStamp, savePath);
}

template <typename T1>
void Export::exportThread(const int frameIndex, const cv::Mat frame,
                          const std::vector<int> ndiHandle, const std::map<int, T1> ndiData,
                          const QString timeStamp, const QString savePath) const {
    QString frameIndexStr = QString::number(frameIndex);
    while (frameIndexStr.length() < 6) {
        frameIndexStr = "0" + frameIndexStr;
    }
    createFolder(savePath);
    //
    if (!frame.empty())
        saveFrame(frameIndexStr, frame, savePath);
    //
    QString filePath = savePath + "/PoseWithTimeStamp/" + frameIndexStr + ".json";
    savePoseWithTimeStamp<T1>(ndiHandle, ndiData, timeStamp, filePath);
}

template <typename T>
void Export::savePoseWithTimeStamp(const std::vector<int> ndiHandle, const std::map<int, T> ndiData, \
                                   const QString timeStamp, const QString filePath) const {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        QString temp;
        out << "{\n";
        out << "\"TimeStamp\": \"" << timeStamp << "\",\n";
        //
        out << "\"Poses\" : \n";
            out << "\t{\n";
            for(int i=0; i<4;i++){
                temp.clear();
                if(ndiHandle[i]>0){
                    T data = ndiData.at(ndiHandle[i]);
                    this->ndiData2QString<T>(data, temp);
                }
                else{
                    temp = "[-1]";
                }
                out << "\t\"" + QString::number(i) +"\":" + temp;
                if (i == 3)
                    out << "\n";
                else
                    out << ",\n";
            }
            out << "\t}\n";
        //
        out << "} \n";
        file.close();
    }
}


#endif // EXPORT_H
