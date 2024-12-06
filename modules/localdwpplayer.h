#ifndef LOCALDWPPLAYER_H
#define LOCALDWPPLAYER_H

#include <io.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include "../3rdparty/Aurora/APIStructures.h"

using data_ptr7 = std::shared_ptr<QuatTransformationStruct>;

class LocalDwPPlayer
{
public:
    LocalDwPPlayer();
    ~LocalDwPPlayer();
    bool loadDwP(const std::string &dwpPath);
    bool unloadDwP();
    void setRecurrence(bool ifRecurrent);
    void setFPS(float fps); // 设置帧率, 计算出每帧等待时间
    void jumpTo(int idx);
    bool isReadyToGrab(); // 是否准备好被抓取
    void flipFlag(); // 播放/暂停
    int getLength();
    int getNowIdx();
    bool getOneFrame(cv::Mat &frame,
                     std::vector<int> &ndiHandle,
                     std::map<int, data_ptr7> &poseMap,
                     std::map<int, double> &poseMapErr); //poseMap: port name -> port value
    bool isPlaying();
private:
    std::string dwpPath;
    int _length;        // 总帧数
    bool _ifDwPLegal(const std::string &dwpPath); // 判断dwp文件是否合法(文件夹存在，编号连续)
    bool _ifLoaded;     // 是否已加载, loaded == ready
    int _nowIdx;         // 当前帧序号
    bool _ifRecurrent;  // 是否循环播放
    float _waitMS;      // 每帧等待时间
    bool _ifPlaying;    // 是否正在播放（播放后，getOneFrame会前进；否则，getOneFrame不前进）
    std::vector<int> ndiHandle;  //idx->port name, size == 4; 
};

#endif // LOCALDWPPLAYER_H
