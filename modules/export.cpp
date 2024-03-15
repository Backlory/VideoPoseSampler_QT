#include "export.h"

Export::Export()
{

}

Export::~Export()
{

}




// =======================
template <typename T>
void Export::exportData(const int frameIndex, const cv::Mat frame, const std::map<int, T> ndiData, \
    QString timeStamp, QString savePath) const {
    exportThread<T>(frameIndex, frame, ndiData, timeStamp, savePath);
    //boost::thread exportThread(&Export::exportThread, this, frameIndex, frame, ndiData, timeStamp, savePath);
    //exportThread.join();
}

template <typename T1>
void Export::exportThread(const int frameIndex, const cv::Mat frame, const std::map<int, T1> ndiData,
                  const QString timeStamp, const QString savePath) const {
    QString frameIndexStr = QString::number(frameIndex);
    while (frameIndexStr.length() < 6) {
        frameIndexStr = "0" + frameIndexStr;
    }
    createFolder(savePath);
    saveFrame(frameIndexStr, frame, savePath);
    savePoseWithTimeStamp<T1>(frameIndexStr, ndiData, timeStamp, savePath);
}


template <typename T>
void Export::savePoseWithTimeStamp(const QString frameIndexStr, const std::map<int, T> ndiData, \
            const QString timeStamp, const QString savePath) const {
    QString filePath = savePath + "/PoseWithTimeStamp/" + frameIndexStr + ".json";
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "{\n";
        out << "TimeStamp: " << timeStamp << "\n";
        for (auto it = ndiData.begin(); it != ndiData.end(); it++) {
            out << "Position Sensor: " << it->first << "\n";
            out << "\t{\n";
            out << "T: " << it->second->position << "\n";
            out << "R: " << it->second->orientation << "\n";
            out << "\t} \n";
        }
        out << "} \n";
        file.close();
    }

}

// =======================
void Export::createFolder(const QString savePath) const{
    QDir dir;
    if (!dir.exists(savePath)) {
        dir.mkpath(savePath);
        dir.mkpath(savePath + "/frame");
        dir.mkpath(savePath + "/PoseWithTimeStamp");
    }
}

void Export::saveFrame(const QString frameIndexStr, const cv::Mat frame, const QString savePath) const{
    QString filePath = savePath + "/frame/" + frameIndexStr + ".jpg";
    cv::imwrite(filePath.toStdString(), frame);
}


void Export::d7to4(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr4> &d4) {
    for (auto it = d7.begin(); it != d7.end(); it++) {
        auto id = it->first;
        auto ptr = it->second;
        if (ptr) {
            Eigen::Vector3d pos(ptr->translation.x, ptr->translation.y, ptr->translation.z);
            Eigen::Quaterniond rot(ptr->rotation.q0, ptr->rotation.qx, ptr->rotation.qy, ptr->rotation.qz);
            Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
            transformation.block<3,3>(0,0) = rot.toRotationMatrix();
            transformation.block<3,1>(0,3) = pos;
            auto ptr4 = std::make_shared<Eigen::Matrix4d>(transformation);
            d4.emplace(id, ptr4);
        }
        else {
            d4.emplace(id, nullptr);
        }
    }
}

void Export::d7to6(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr6> &d6) {
    for (auto it = d7.begin(); it != d7.end(); it++) {
        auto id = it->first;
        auto ptr = it->second;
        if (ptr) {
            Eigen::Vector3d pos(ptr->translation.x, ptr->translation.y, ptr->translation.z);
            Eigen::Quaterniond rot(ptr->rotation.q0, ptr->rotation.qx, ptr->rotation.qy, ptr->rotation.qz);
            rot.normalize();
            Eigen::Vector3d lie = Sophus::SO3d::SO3(rot.toRotationMatrix()).log();
            auto ptr6 = std::make_shared<Sophus::Vector6d>((Sophus::Vector6d() << pos, lie).finished());
            d6.emplace(id, ptr6);
        }
        else {
            d6.emplace(id, nullptr);
        }
    }
}
