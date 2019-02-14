#-------------------------------------------------
#
# Project created by QtCreator 2019-02-05T19:55:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DesktopPatternMatcher
TEMPLATE = app


SOURCES += main.cpp\
    DPM_MainWindow.cpp \
    DPM_ImagePatternSelector.cpp \
    DPM_ScreenShotPatternMatcher.cpp

HEADERS  += \
    DPM_ImagePatternSelector.h \
    DPM_MainWindow.h \
    DPM_ScreenShotPatternMatcher.h

FORMS    += \
    DPM_MainWindow.ui \
    DPM_ImagePatternSelector.ui

LIBS += -lUser32

RESOURCES += \
    res.qrc

DISTFILES += \
    res.rc

RC_FILE += \
	res.rc
