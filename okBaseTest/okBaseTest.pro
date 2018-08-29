#-------------------------------------------------
#
# Project created by QtCreator 2018-02-24T14:04:33
#
#-------------------------------------------------

QT       += core gui multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = okBaseTest
TEMPLATE = app

linux-g++ {
    contains(QT_ARCH, i386) {
        LIBS += -L$$PWD/lib/linux_x86 -lokFrontPanel -lCelexSensor }
    else {
        LIBS += -L$$PWD/lib/linux_x64 -lokFrontPanel -lCelexSensor }
}
win32 {
    contains(QT_ARCH, i386) {
        LIBS += -L$$PWD/lib/windows_x86/ -lCelexSensor }
    else {
        LIBS += -L$$PWD/lib/windows_x64/ -lCelexSensor }
}

SOURCES += main.cpp\
    mainwindow.cpp \
    dataqueue.cpp \
    doubleslider.cpp \
    hhsliderwidget.cpp

HEADERS  += mainwindow.h \
    celexsensordll.h \
    okFrontPanelDLL.h \
    dataqueue.h \
    doubleslider.h \
    hhsliderwidget.h \
    hhconstants.h \
    ui_mainwindow.h

FORMS += \
    mainwindow.ui

win32 {
    CONFIG(debug) {
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.xml $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\FPN.txt $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.bit $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.xml $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\FPN.txt $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.bit $$shell_path($$OUT_PWD)\release\
    }
    else {
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.xml $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\FPN.txt $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.bit $$shell_path($$OUT_PWD)\debug\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.xml $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.dll $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\FPN.txt $$shell_path($$OUT_PWD)\release\ &
        QMAKE_POST_LINK += copy $$shell_path($$PWD)\*.bit $$shell_path($$OUT_PWD)\release\
    }
}

linux-g++ {
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/*.xml $$shell_path($$OUT_PWD)/ &
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/FPN.txt $$shell_path($$OUT_PWD)/ &
    QMAKE_POST_LINK += cp $$shell_path($$PWD)/*.bit $$shell_path($$OUT_PWD)/
}

RESOURCES += \
    images.qrc
