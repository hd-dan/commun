TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    commun.cpp \
    serialcom.cpp \
    ../ur_robot/ur/joystick.cpp

HEADERS += \
    commun.h \
    serialcom.h \
    ../ur_robot/ur/joystick.h \
    ../ur_robot/ur/util.hpp

LIBS += -pthread -lboost_thread -lboost_iostreams -lboost_system -lutil -lrt
