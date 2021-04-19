QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets svg
qtHaveModule(opengl): QT += opengl


CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/graph.cpp \
    core/manager.cpp \
    core/viewer.cpp \
    core/main.cpp \
    device/map.cpp \
    device/mainwindow.cpp \
    device/pathwidget.cpp \
    shops_data.cpp

HEADERS += \
    device/graph.h \
    device/manager.h \
    device/map.h \
    device/pathwidget.h \
    device/viewer.h \
    device\mainwindow.h \
    shops_data.h

FORMS += \
    ../resources/PathWidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../resources/map1(2).qrc \
    ../resources/map1.qrc \
    ../resources/pics.qrc \
