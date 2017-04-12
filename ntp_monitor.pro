TEMPLATE = app

QT += core gui widgets
CONFIG += c++11
CONFIG(debug, release|debug):DEFINES += DEBUG

SOURCES += main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.hpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Default rules for deployment.
unix:!android: target.path = /opt/$${TARGET}
!isEmpty(target.path): INSTALLS += target
