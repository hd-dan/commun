TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    commun.cpp \
    serialcom.cpp \
    ../util/joystick.cpp

HEADERS += \
    commun.h \
    serialcom.h \
    ../util/joystick.h

LIBS += -pthread -lboost_thread -lboost_iostreams -lboost_system -lutil -lrt
