#-------------------------------------------------
#
# Project created by QtCreator 2012-05-03T14:20:22
#
#-------------------------------------------------

LIBS += -lGLESv2 -lEGL

QT       += core gui opengl
# QT += meegographicssystemhelper

TARGET = fluid
TEMPLATE = app


SOURCES += main.cpp\
    glwindow.cpp

HEADERS  += \
    glwindow.h

CONFIG += mobility
MOBILITY = 

symbian {
    TARGET.UID3 = 0xe2bb6c0a
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog \
    simple.vsh \
    eval.fsh \
    render.fsh \
    Trees.jpg

contains(MEEGO_EDITION,harmattan) {
    target.path = /opt/fluid/bin
    INSTALLS += target

    shaders.path = /opt/fluid/bin
    shaders.files += simple.vsh render.fsh eval.fsh Trees.jpg
    INSTALLS += shaders
}
