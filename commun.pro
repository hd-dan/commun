TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    commun.cpp

HEADERS += \
    commun.h

LIBS += -pthread -lboost_thread -lboost_iostreams -lboost_system -lutil -lrt
