QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addbutton.cpp \
    animations/animationdialog.cpp \
    animations/animationform.cpp \
    exportdialog.cpp \
    fileio.cpp \
    main.cpp \
    mainwindow.cpp \
    palette.cpp \
    sprite.cpp \
    spriteview.cpp

HEADERS += \
    addbutton.h \
    animations/animationdialog.h \
    animations/animationform.h \
    exportdialog.h \
    fileio.h \
    mainwindow.h \
    palette.h \
    sprite.h \
    spriteview.h

FORMS += \
    animations/animationdialog.ui \
    animations/animationform.ui \
    exportdialog.ui \
    mainwindow.ui


RC_ICONS = icons/opensprite96x96.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
