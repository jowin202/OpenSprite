QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# application version (single source: used in the About dialog and file properties)
VERSION = 2.0.0
VERSION_DATE = 11/2026
DEFINES += APP_VERSION=\\\"$$VERSION\\\" APP_VERSION_DATE=\\\"$$VERSION_DATE\\\"

CONFIG(debug, debug|release) {
    DESTDIR = build/debug
} else {
    DESTDIR = build/release
}

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
    quickeditor.cpp \
    rotations/rotationdialog.cpp \
    scale/scalingdialog.cpp \
    seriesutil.cpp \
    settingsdialog.cpp \
    sprite.cpp \
    spritedata.cpp \
    spriteview.cpp

HEADERS += \
    addbutton.h \
    animations/animationdialog.h \
    animations/animationform.h \
    exportdialog.h \
    fileio.h \
    mainwindow.h \
    palette.h \
    quickeditor.h \
    rotations/rotationdialog.h \
    scale/scalingdialog.h \
    seriesutil.h \
    settingsdialog.h \
    sprite.h \
    spritedata.h \
    spriteview.h

FORMS += \
    animations/animationdialog.ui \
    animations/animationform.ui \
    exportdialog.ui \
    mainwindow.ui \
    rotations/rotationdialog.ui \
    scale/scalingdialog.ui \
    settingsdialog.ui


RC_ICONS = icons/opensprite96x96.ico


# macOS icon
macx {
    CONFIG += app_bundle
    ICON = icons/icon.icns
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
