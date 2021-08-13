SOURCES += \
	main.cpp \
	../parser.cpp \
	../terminal.cpp \
	../textrender.cpp \
	../ptyiface.cpp \
	../utilities.cpp

HEADERS += \
	../parser.h \
	../terminal.h \
	../textrender.h \
	../ptyiface.h \
	../utilities.h \
	../catch.hpp

INCLUDEPATH += ..
DEPENDPATH += ..

DEFINES += TEST_MODE
QT += quick testlib
LIBS += -lutil
