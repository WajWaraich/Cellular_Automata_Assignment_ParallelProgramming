#-------------------------------------------------
#
# Project created by QtCreator 2016-03-08T13:35:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GameOfLife
TEMPLATE = app

QMAKE_LFLAGS += /STACK:32000000

QMAKE_CXXFLAGS += -openmp -arch:AVX -D "_CRT_SECURE_NO_WARNINGS"
QMAKE_CXXFLAGS_RELEASE *= -O2

SOURCES += main.cpp\
        mainwindow.cpp

INCLUDEPATH += C:\\opencv310\\build\\install\\include

LIBS += -LC:\\opencv310\\build\\install\\x86\\vc11\\lib\
    -lopencv_bgsegm310d\
    -lopencv_bioinspired310d\
    -lopencv_calib3d310d\
    -lopencv_ccalib310d\
    -lopencv_core310d\
    -lopencv_datasets310d\
    -lopencv_dnn310d\
    -lopencv_dpm310d\
    -lopencv_face310d\
    -lopencv_features2d310d\
    -lopencv_flann310d\
    -lopencv_highgui310d\
    -lopencv_imgcodecs310d\
    -lopencv_imgproc310d\
    -lopencv_line_descriptor310d\
    -lopencv_ml310d\
    -lopencv_objdetect310d\
    -lopencv_photo310d\
    -lopencv_reg310d\
    -lopencv_rgbd310d\
    -lopencv_saliency310d\
    -lopencv_shape310d\
    -lopencv_stereo310d\
    -lopencv_structured_light310d\
    -lopencv_superres310d\
    -lopencv_surface_matching310d\
    -lopencv_text310d\
    -lopencv_tracking310d\
    -lopencv_ts310d\
    -lopencv_video310d\
    -lopencv_videoio310d\
    -lopencv_videostab310d\
    -lopencv_xobjdetect310d\

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
