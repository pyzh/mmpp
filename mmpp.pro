
# Comment this to enable QT
CONFIG -= qt

CONFIG(qt) {
    QT       += core gui
    greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
    DEFINES += USE_QT
}

TEMPLATE = app
CONFIG += link_pkgconfig
CONFIG += precompile_header
#CONFIG += object_parallel_to_source
PKGCONFIG += libmicrohttpd libcrypto++

SOURCES += \
    main.cpp \
    library.cpp \
    proof.cpp \
    old/unification.cpp \
    wff.cpp \
    toolbox.cpp \
    test/test.cpp \
    utils/utils.cpp \
    web/web.cpp \
    platform.cpp \
    web/workset.cpp \
    web/jsonize.cpp \
    z3/z3prover.cpp \
    reader.cpp \
    apps/unificator.cpp \
    temp.cpp \
    test/test_env.cpp \
    apps/generalizable_theorems.cpp \
    test/test_parsing.cpp \
    test/test_verification.cpp \
    test/test_minor.cpp \
    web/httpd_microhttpd.cpp \
    web/step.cpp \
    utils/threadmanager.cpp \
    apps/learning.cpp

CONFIG(qt) {
SOURCES += \
    qt/mainwindow.cpp \
    qt/prooftreemodel.cpp \
    qt/htmldelegate.cpp \
    qt/main_qt.cpp
}

# Compile with gcc
QMAKE_CC = gcc
QMAKE_CXX = g++
QMAKE_LINK = g++
QMAKE_CFLAGS += -std=c11 -g
QMAKE_CXXFLAGS += -std=c++17 -g -ftemplate-backtrace-limit=0
QMAKE_LIBS += -ldl -export-dynamic -rdynamic -lboost_system -lboost_filesystem -lboost_serialization -lpthread -lz3

# Compile to native instruction set
#QMAKE_CFLAGS += -march=native
#QMAKE_CXXFLAGS += -march=native

# Compile with clang
#QMAKE_CC = clang-5.0
#QMAKE_CXX = clang++-5.0
#QMAKE_LINK = clang++-5.0
#QMAKE_CFLAGS += -std=c11 -march=native -g
#QMAKE_CXXFLAGS += -std=c++17 -march=native -g -ftemplate-backtrace-limit=0
#QMAKE_LIBS += -ldl -rdynamic -lboost_system -lboost_filesystem -lboost_serialization -lpthread -lz3

# Disable these to have faster code; enable them to spot bugs
#QMAKE_CXXFLAGS += -DLR_PARSER_SELF_TEST
#QMAKE_CXXFLAGS += -DUNIFICATOR_SELF_TEST
#QMAKE_CXXFLAGS += -DTOOLBOX_SELF_TEST

# Experiments with undefined behavior checking
#QMAKE_CFLAGS += -fsanitize=undefined -fno-sanitize-recover=all
#QMAKE_CXXFLAGS += -fsanitize=undefined -fno-sanitize-recover=all
#QMAKE_LIBS += -fsanitize=undefined -fno-sanitize-recover=all

# Experiments with address checking
#QMAKE_CFLAGS += -fsanitize=address -fno-sanitize-recover=all -fsanitize-address-use-after-scope
#QMAKE_CXXFLAGS += -fsanitize=address -fno-sanitize-recover=all -fsanitize-address-use-after-scope
#QMAKE_LIBS += -fsanitize=address -fno-sanitize-recover=all -fsanitize-address-use-after-scope

PRECOMPILED_HEADER += pch.h

HEADERS += \
    pch.h \
    wff.h \
    library.h \
    proof.h \
    old/unification.h \
    toolbox.h \
    utils/stringcache.h \
    utils/utils.h \
    web/httpd.h \
    web/web.h \
    libs/json.h \
    platform.h \
    web/workset.h \
    web/jsonize.h \
    reader.h \
    libs/serialize_tuple.h \
    test/test_env.h \
    test/test_parsing.h \
    test/test_verification.h \
    test/test_minor.h \
    utils/vectormap.h \
    parsing/parser.h \
    parsing/earley.h \
    parsing/lr.h \
    parsing/unif.h \
    web/httpd_microhttpd.h \
    web/step.h \
    utils/threadmanager.h \
    utils/backref_registry.h \
    parsing/algos.h

CONFIG(qt) {
HEADERS += \
    qt/mainwindow.h \
    qt/prooftreemodel.h \
    qt/htmldelegate.h
}

DISTFILES += \
    README

FORMS += \
    qt/mainwindow.ui

#create_links.commands = for i in mmpp_dissector mmpp_gen_random_theorems mmpp_verify_one mmpp_simple_verify_one mmpp_verify_all mmpp_test_setmm mmpp_unificator webmmpp webmmpp_open qmmpp mmpp_test_z3 mmpp_generalizable_theorems ; do ln -s mmpp \$\$i 2>/dev/null || true ; done
#QMAKE_EXTRA_TARGETS += create_links
#POST_TARGETDEPS += create_links
