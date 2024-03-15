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
        ../3rdparty/OpenCV/x64/vc15/lib/*d.lib \
        ../3rdparty/Boost/lib/*-mt-gd-x64-1_74.lib
}else{
    LIBS += \
        ../3rdparty/OpenCV/x64/vc15/lib/*.lib \
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
    modules/Configloading.cpp \
    modules/cameradetection.cpp \
    modules/comportdetection.cpp \
    modules/delaycalibration.cpp \
    modules/export.cpp \
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
    modules/Configloading.h \
    modules/cameradetection.h \
    modules/comportdetection.h \
    modules/delaycalibration.h \
    modules/export.h \
    modules/ndimodule.h \
    modules/opencvframe.h \
    modules/timestamp.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    VideoPoseSampler_QT_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
