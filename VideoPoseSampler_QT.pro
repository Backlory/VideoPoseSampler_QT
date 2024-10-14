QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    ../3rdparty/OpenCV/include \
    ../3rdparty/Boost/include/boost-1_73 \
    ../3rdparty/eigen/include/eigen3 \
    ../3rdparty/sophus \
    ../3rdparty/fmt-7.1.3/include

CONFIG(debug, debug|release) {
    LIBS += \
         ../3rdparty/Boost/lib/*-mt-gd-x64-1_74.lib\
         ../3rdparty/OpenCV/x64/vc15/lib/opencv_world450d.lib
}else{
    LIBS += \
        ../3rdparty/OpenCV/x64/vc15/lib/opencv_world450.lib \
        ../3rdparty/Boost/lib/*-mt-x64-1_74.lib
}


SOURCES += \
    ../3rdparty/Aurora/Comm32.cpp \
    ../3rdparty/Aurora/CommandConstruction.cpp \
    ../3rdparty/Aurora/CommandHandling.cpp \
    ../3rdparty/Aurora/Conversions.cpp \
    ../3rdparty/Aurora/INIFileRW.cpp \
    ../3rdparty/Aurora/SystemCRC.cpp \
    ../3rdparty/fmt-7.1.3/src/format.cc \
    ../3rdparty/fmt-7.1.3/src/os.cc \
    main.cpp \
    mainwindow.cpp \
    modules/cameradetection.cpp \
    modules/comportdetection.cpp \
    modules/export.cpp \
    modules/highresolutiontime.cpp \
    modules/ndimodule.cpp \
    modules/opencvframe.cpp \
    modules/timestamp.cpp

HEADERS += \
    ../3rdparty/Aurora/APIStructures.h \
    ../3rdparty/Aurora/Comm32.h \
    ../3rdparty/Aurora/CommandHandling.h \
    ../3rdparty/Aurora/Conversions.h \
    ../3rdparty/Aurora/INIFileRW.h \
    main.h \
    mainwindow.h \
    modules/cameradetection.h \
    modules/comportdetection.h \
    modules/export.h \
    modules/highresolutiontime.h \
    modules/ndimodule.h \
    modules/opencvframe.h \
    modules/timestamp.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    VideoPoseSampler_QT_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

RC_ICONS = assets/vps.ico # ico

CONFIG += file_copies
adddir_d.files = $$PWD/assets
adddir_d.files += $$PWD/config
adddir_d.path = $$OUT_PWD/debug
COPIES += adddir_d
adddir_r.files = $$PWD/assets
adddir_r.files += $$PWD/config
adddir_r.path = $$OUT_PWD/release
COPIES += adddir_r

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
