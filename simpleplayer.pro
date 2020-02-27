TEMPLATE = app
QT   += core gui network

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += avwidgets
  QT += widgets
} else {
  CONFIG += avwidgets
}

CONFIG += c++11
LIBS += -L$$PWD/"3rdparty/qtav/lib" -llibQtAV1 -llibQtAVWidgets1
include(src.pri)

FORMS += \
    customwall.ui

HEADERS += \
    customwall.h \
    stable.h

SOURCES += \
    customwall.cpp

RESOURCES += \
    qrc.qrc


