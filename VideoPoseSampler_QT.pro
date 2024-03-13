QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    ../3rdparty/OpenCV/include
LIBS += \
    ../3rdparty/OpenCV/x64/vc15/lib/*.lib


SOURCES += \
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
