#-------------------------------------------------
#
# Project created by QtCreator 2022-01-15T19:32:59
#
#-------------------------------------------------

QT       += core gui \
    widgets
QT         +=   multimedia
 QT         +=  multimediawidgets webenginewidgets
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = xwcplayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/ffmpeglib/include
INCLUDEPATH += $$PWD/player/include/ffmpeg
INCLUDEPATH += $$PWD/player/include/tutk
INCLUDEPATH += $$PWD/player/extension

#加入FFmpeg链接库
LIBS += $$PWD/ffmpegliba/lib/avcodec.lib   \
        $$PWD/ffmpegliba/lib/avdevice.lib  \
        $$PWD/ffmpegliba/lib/avfilter.lib  \
        $$PWD/ffmpegliba/lib/avformat.lib  \
        $$PWD/ffmpegliba/lib/avutil.lib    \
        $$PWD/ffmpegliba/lib/postproc.lib  \
        $$PWD/ffmpegliba/lib/swresample.lib\
        $$PWD/ffmpegliba/lib/swscale.lib   \
        $$PWD/tutk/AVAPIs.lib \
        $$PWD/tutk/IOTCAPIs.lib


SOURCES += main.cpp\
    camera.cpp \
    mainwindow.cpp \
    player/ac_buf.cpp \
    player/decoder.cpp \
    player/error.cpp \
    player/p2preader.cpp \
    player/resample_audio.cpp \
    yuv_render.cpp \
    playerpanel.cpp \
    widget.cpp \
    fdecode.cpp \
    playwidget.cpp

HEADERS  += widget.h \
    camera.h \
    fdecode.h \
    mainwindow.h \
    playerpanel.h \
    playwidget.h \
    yuv_render.h

FORMS    += widget.ui \
    mainwindow.ui \
    playerpanel.ui

DISTFILES += \
    C:/Users/XWC/Desktop/icon.jpeg

RESOURCES += \
    ../../qrc/icon.qrc
