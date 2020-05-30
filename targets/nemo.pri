desktopfile.extra = cp $${TARGET}.desktop.nemo $${TARGET}.desktop
desktopfile.path = /usr/share/applications
desktopfile.files = $${TARGET}.desktop
INSTALLS += desktopfile
DEFINES += MOBILE_BUILD
DEFAULT_DRAG_MODE = scroll

qtHaveModule(feedback): {
    QT += feedback
    DEFINES += HAVE_FEEDBACK
}
