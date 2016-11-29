TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    parser.cpp \
    library.cpp \
    proof.cpp \
    unification.cpp \
    memory.c

QMAKE_CXXFLAGS += -std=c++17 -march=native
QMAKE_LIBS += -ldl -export-dynamic -rdynamic -lboost_system -lboost_filesystem

HEADERS += \
    wff.h \
    parser.h \
    library.h \
    statics.h \
    proof.h \
    unification.h \
    memory.h
