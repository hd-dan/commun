TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
        main.cpp \
    serialcom.cpp \
#    ../util/joystick.cpp
    tcpcom.cpp \
    btcom.cpp

HEADERS += \
    serialcom.h \
#    ../util/joystick.h
    tcpcom.h \
    btcom.h


macx {
    INCLUDEPATH += /usr/local/Cellar/boost/1.70.0/include/
    LIBS += -L/usr/local/Cellar/boost/1.70.0/lib -lboost_thread-mt
}
unix:!macx {
    LIBS += -lboost_thread -lrt
}
LIBS += -pthread -lutil -lboost_iostreams -lboost_system -lgattlib

