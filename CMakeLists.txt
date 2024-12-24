cmake_minimum_required(VERSION 3.5)

project(VideoPoseSampler_QT VERSION 1.3.5 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets SerialPort)


include_directories(
    ../3rdparty/OpenCV/include
    ../3rdparty/Boost/include/boost-1_73
    ../3rdparty/eigen/include/eigen3
    ../3rdparty/sophus
    ../3rdparty/fmt-7.1.3/include
)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
# Define source and header files
set(PROJECT_SOURCES
    ../3rdparty/Aurora/Comm32.cpp
    ../3rdparty/Aurora/CommandConstruction.cpp
    ../3rdparty/Aurora/CommandHandling.cpp
    ../3rdparty/Aurora/Conversions.cpp
    ../3rdparty/Aurora/INIFileRW.cpp
    ../3rdparty/Aurora/SystemCRC.cpp
    ../3rdparty/fmt-7.1.3/src/format.cc
    ../3rdparty/fmt-7.1.3/src/os.cc
    main.cpp
    mainwindow.cpp
    modules/cameradetection.cpp
    modules/comportdetection.cpp
    modules/export.cpp
    modules/localdwpplayer.cpp
    modules/ndimodule.cpp
    modules/opencvframe.cpp
    modules/timestamp.cpp
    modules/highresolutiontime.cpp
    selectresolution.cpp

    ../3rdparty/Aurora/APIStructures.h
    ../3rdparty/Aurora/Comm32.h
    ../3rdparty/Aurora/CommandHandling.h
    ../3rdparty/Aurora/Conversions.h
    ../3rdparty/Aurora/INIFileRW.h
    main.h
    mainwindow.h
    modules/cameradetection.h
    modules/comportdetection.h
    modules/export.h
    modules/localdwpplayer.h
    modules/ndimodule.h
    modules/opencvframe.h
    modules/timestamp.h
    modules/highresolutiontime.h
    selectresolution.h

    mainwindow.ui
    selectresolution.ui
    ${app_icon_resource_windows}
)



# ====================================================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::SerialPort
)

# Add debug and release specific libraries, include directories
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_thread-vc142-mt-gd-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_chrono-vc142-mt-gd-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_date_time-vc142-mt-gd-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_filesystem-vc142-mt-gd-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/OpenCV/x64/vc15/lib/opencv_world450d.lib"
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_thread-vc142-mt-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_chrono-vc142-mt-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_date_time-vc142-mt-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/Boost/lib/libboost_filesystem-vc142-mt-x64-1_74.lib"
        "${CMAKE_SOURCE_DIR}/../3rdparty/OpenCV/x64/vc15/lib/opencv_world450.lib"
    )
endif()


# ===================================================

# Add resources
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/debug)
else()
    set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/release)
endif()
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config)
#file(COPY ${ASSETS_DIR} DESTINATION ${OUTPUT_DIR})
#file(COPY ${CONFIG_DIR} DESTINATION ${OUTPUT_DIR})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION ./bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/bin
)

install(DIRECTORY ${ASSETS_DIR} DESTINATION ${CMAKE_INSTALL_BINDIR}/bin)
install(DIRECTORY ${CONFIG_DIR} DESTINATION ${CMAKE_INSTALL_BINDIR}/bin)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
