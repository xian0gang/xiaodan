#-------------------------------------------------
#
# Project created by QtCreator 2018-09-26T11:00:08
#
#-------------------------------------------------

QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = xiaodan
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    appinit.cpp \
    iconhelper.cpp \
    commonhelper.cpp \
    cpacketlist.cpp \
    readthread.cpp \
    mythread.cpp \
    showlabel.cpp

HEADERS  += widget.h \
    appinit.h \
    iconhelper.h \
    commonhelper.h \
    cpacketlist.h \
    readthread.h \
    mythread.h \
    showlabel.h

FORMS    += widget.ui

RESOURCES += \
    qss.qrc \
    image.qrc


INCLUDEPATH     +=  D:\FFMPEG\dev\include
LIBS            +=  -LD:\FFMPEG\dev\lib -lavcodec     \
                    -LD:\FFMPEG\dev\lib -lavdevice     \
                    -LD:\FFMPEG\dev\lib -lavfilter     \
                    -LD:\FFMPEG\dev\lib -lavformat     \
                    -LD:\FFMPEG\dev\lib -lavutil       \
                    -LD:\FFMPEG\dev\lib -lpostproc     \
                    -LD:\FFMPEG\dev\lib -lswscale



DEFINES += __MW_STDINT_H__

DISTFILES += \
    myico.rc \

RC_FILE += myico.rc
