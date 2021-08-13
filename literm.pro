QT = core gui qml quick

CONFIG -= app_bundle

MOC_DIR = .moc
OBJECTS_DIR = .obj

CONFIG += link_pkgconfig

TEMPLATE = app
TARGET = literm
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lutil

# Input
HEADERS += \
    ptyiface.h \
    terminal.h \
    textrender.h \
    version.h \
    utilities.h \
    keyloader.h \
    parser.h \
    catch.hpp

SOURCES += \
    main.cpp \
    terminal.cpp \
    textrender.cpp \
    ptyiface.cpp \
    utilities.cpp \
    keyloader.cpp \
    parser.cpp

OTHER_FILES += \
    qml/mobile/Main.qml \
    qml/mobile/Keyboard.qml \
    qml/mobile/Key.qml \
    qml/mobile/Lineview.qml \
    qml/mobile/Button.qml \
    qml/mobile/MenuLiterm.qml \
    qml/mobile/NotifyWin.qml \
    qml/mobile/UrlWindow.qml \
    qml/mobile/LayoutWindow.qml \
    qml/mobile/PopupWindow.qml

OTHER_FILES += \
    qml/desktop/Main.qml \
    qml/desktop/Keyboard.qml \
    qml/desktop/Key.qml \
    qml/desktop/Button.qml \
    qml/desktop/MenuLiterm.qml \
    qml/desktop/NotifyWin.qml \
    qml/desktop/UrlWindow.qml \
    qml/desktop/LayoutWindow.qml \
    qml/desktop/PopupWindow.qml \
    qml/desktop/TabView.qml \
    qml/desktop/TabBar.qml

RESOURCES += \
    resources.qrc

target.path = /usr/bin
INSTALLS += target

isEmpty(LITERM_TARGET): LITERM_TARGET=desktop
!include(targets/$${LITERM_TARGET}.pri): error("Can't load LITERM_TARGET definition: $$LITERM_TARGET")
