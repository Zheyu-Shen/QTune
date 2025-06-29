TARGET = qtune
QT       += core gui
QT       += core gui multimedia
QT       += core gui charts
QT       += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
DEFINES += PROJECT_SOURCE_DIR=\\\"$$PWD\\\"
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aiassistant.cpp \
    drawscore.cpp \
    generatescore.cpp \
    listen.cpp \
    listensub.cpp \
    listensubsub.cpp \
    log.cpp \
    main.cpp \
    homepage.cpp \
    noteaudiosource.cpp \
    playscore.cpp \
    sing.cpp \
    singsub.cpp \
    write.cpp \
    writesub.cpp\
    midifile/MidiFile.cpp \
    midifile/MidiEvent.cpp \
    midifile/MidiEventList.cpp \
    midifile/MidiMessage.cpp \
    midifile/Binasc.cpp \
    midifile/Options.cpp \
    Gist/Gist.cpp \
    Gist/kiss_fft.c \
    Gist/AccelerateFFT.cpp \
    Gist/CoreFrequencyDomainFeatures.cpp \
    Gist/CoreTimeDomainFeatures.cpp \
    Gist/MFCC.cpp \
    Gist/OnsetDetectionFunction.cpp \
    Gist/WindowFunctions.cpp \
    Gist/Yin.cpp \

HEADERS += \
    aiassistant.h \
    drawscore.h \
    generatescore.h \
    homepage.h \
    listen.h \
    listensub.h \
    listensubsub.h \
    log.h \
    note.h \
    noteaudiosource.h \
    playscore.h \
    sing.h \
    singsub.h \
    write.h \
    writesub.h\
    note.h\
    midifile/MidiFile.h \
    midifile/MidiEvent.h \
    midifile/MidiEventList.h \
    midifile/MidiMessage.h \
    midifile/Binasc.h \
    midifile/Options.h \
    Gist/Gist.h \
    Gist/AccelerateFFT.h \
    Gist/CoreFrequencyDomainFeatures.h \
    Gist/CoreTimeDomainFeatures.h \
    Gist/MFCC.h \
    Gist/OnsetDetectionFunction.h \
    Gist/WindowFunctions.h \
    Gist/Yin.h \

FORMS += \
    aiassistant.ui \
    homepage.ui \
    listen.ui \
    listensub.ui \
    listensubsub.ui \
    log.ui \
    sing.ui \
    singsub.ui \
    write.ui \
    writesub.ui

TRANSLATIONS += \
    Qtune_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    picture.qrc
