/*
 * @Author: backlory's desktop dbdx_liyaning@126.com
 * @Date: 2024-03-16 13:24:52
 * @LastEditors: backlory's desktop dbdx_liyaning@126.com
 * @LastEditTime: 2024-03-16 18:36:29
 * @Description: 
 * Copyright (c) 2024 by Backlory, (email: dbdx_liyaning@126.com), All Rights Reserved.
 */
#include "export.h"

Export::Export()
{

}

Export::~Export()
{

}




// =======================

using data_ptr4 = std::shared_ptr<Eigen::Matrix4d>;
using data_ptr6 = std::shared_ptr<Sophus::Vector6d>;
using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;


//将Eigen::Matrix4d转换为QString
//[[a11, a12, a13, a14]\n, [a21, a22, a23, a24]\n, [a31, a32, a33, a34]\n, [a41, a42, a43, a44]]\n
template<>
void Export::ndiData2QString<data_ptr4>(data_ptr4 mat, QString & str) const{
    auto i2s = [&mat](int i, int j){return QString::number((*mat)(i,j));};
    if(mat){
        str += "[[" + i2s(0,0) + ", " + i2s(0,1) + ", " + i2s(0,2) + ", " + i2s(0,3) + "],\n";
        str += "[" + i2s(1,0) + ", " + i2s(1,1) + ", " + i2s(1,2) + ", " + i2s(1,3) + "],\n";
        str += "[" + i2s(2,0) + ", " + i2s(2,1) + ", " + i2s(2,2) + ", " + i2s(2,3) + "],\n";
        str += "[" + i2s(3,0) + ", " + i2s(3,1) + ", " + i2s(3,2) + ", " + i2s(3,3) + "]]";
    }
}
//将Sophus::Vector6d转换为QString
template<>
void Export::ndiData2QString<data_ptr6>(data_ptr6 mat, QString & str) const{
    auto i2s = [&mat](int i){return QString::number((*mat)(i));};
    if(mat){
        str += "[" + i2s(0) + ", " + i2s(1) + ", " + i2s(2) + ", " + i2s(3) + ", " + i2s(4) + ", " + i2s(5) + "]";
    }
}

//将QuatTransformationStruct转换为QString
template<>
void Export::ndiData2QString<data_ptr7>(data_ptr7 mat, QString & str) const{
    auto d2s = [](double d){return QString::number(d);};
    if(mat){
        str = "[" + d2s(mat->translation.x) + ", " + d2s(mat->translation.y) + ", " + d2s(mat->translation.z) +
            + ", " + d2s(mat->rotation.q0) + ", " + d2s(mat->rotation.qx) + ", " + d2s(mat->rotation.qy) + ", " + d2s(mat->rotation.qz) + "]";
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

/*
 * d7长度不定，若为某键的值为空，则d4对应值也为空。
*/
void Export::d7to4(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr4> &d4) {
    d4.clear();
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

/*
 * d7长度不定，若为某键的值为空，则d6对应值也为空。
*/
void Export::d7to6(const std::map<int, data_ptr7> &d7, std::map<int, data_ptr6> &d6) {
    d6.clear();
    for (auto it = d7.begin(); it != d7.end(); it++) {
        auto id = it->first;
        auto ptr = it->second;
        if (ptr) {
            try{
                Eigen::Vector3d pos(ptr->translation.x, ptr->translation.y, ptr->translation.z);
                Eigen::Quaterniond rot(ptr->rotation.q0, ptr->rotation.qx, ptr->rotation.qy, ptr->rotation.qz);
                rot.normalize();
                Eigen::Vector3d lie = Sophus::SO3d::SO3(rot.toRotationMatrix()).log();
                auto ptr6 = std::make_shared<Sophus::Vector6d>((Sophus::Vector6d() << pos, lie).finished());
                d6.emplace(id, ptr6);
            }
            catch(...){
                auto ptr6 = std::make_shared<Sophus::Vector6d>((Sophus::Vector6d() << 0, 0, 0, 0, 0, 0).finished());
                d6.emplace(id, ptr6);
            }
        }
        else {
            d6.emplace(id, nullptr);
        }
    }
}
