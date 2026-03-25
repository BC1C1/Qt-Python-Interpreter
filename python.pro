QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core/objects/ast/astnode.cpp \
    core/objects/runtime/environment.cpp \
    core/objects/runtime/pbool.cpp \
    core/objects/runtime/pfloat.cpp \
    core/objects/runtime/pint.cpp \
    core/objects/runtime/piterator.cpp \
    core/objects/runtime/plist.cpp \
    core/objects/runtime/pnone.cpp \
    core/objects/runtime/pstr.cpp \
    core/objects/runtime/pvm.cpp \
    core/utils/compiler.cpp \
    core/utils/functions.cpp \
    core/utils/lexer.cpp \
    core/utils/parser.cpp \
    main.cpp \
    mainwindow.cpp \
    core/objects/runtime/pobject.cpp

HEADERS += \
    core/objects/ast/astnode.h \
    core/objects/runtime/environment.h \
    core/objects/runtime/pbool.h \
    core/objects/runtime/pfloat.h \
    core/objects/runtime/pint.h \
    core/objects/runtime/piterator.h \
    core/objects/runtime/plist.h \
    core/objects/runtime/pnone.h \
    core/objects/runtime/pstr.h \
    core/objects/runtime/pvm.h \
    core/utils/compiler.h \
    core/utils/functions.h \
    core/utils/lexer.h \
    core/utils/parser.h \
    mainwindow.h \
    core/objects/runtime/pobject.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
