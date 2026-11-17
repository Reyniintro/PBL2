QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Include paths for the 5-layer architecture (UI, Domain, Application, Algorithms, Infrastructure)
INCLUDEPATH += \
    $$PWD \
    $$PWD/src \
    $$PWD/src/ui \
    $$PWD/src/ui/pages \
    $$PWD/src/ui/dialogs \
    $$PWD/src/domain \
    $$PWD/src/domain/entities \
    $$PWD/src/domain/enums \
    $$PWD/src/domain/policies \
    $$PWD/src/application \
    $$PWD/src/algorithms \
    $$PWD/src/infrastructure \
    $$PWD/src/infrastructure/repositories

DEPENDPATH += $$INCLUDEPATH

# Legacy core sources (preserved)
SOURCES += \
    CoreEntities.cpp \
    main.cpp \
    mainwindow.cpp

# Automatic discovery of sources in the new 5-layer architecture
SOURCES += \
    $$files($$PWD/src/ui/*.cpp, false) \
    $$files($$PWD/src/ui/pages/*.cpp, false) \
    $$files($$PWD/src/ui/dialogs/*.cpp, false) \
    $$files($$PWD/src/domain/*.cpp, false) \
    $$files($$PWD/src/domain/entities/*.cpp, false) \
    $$files($$PWD/src/domain/enums/*.cpp, false) \
    $$files($$PWD/src/domain/policies/*.cpp, false) \
    $$files($$PWD/src/application/*.cpp, false) \
    $$files($$PWD/src/algorithms/*.cpp, false) \
    $$files($$PWD/src/infrastructure/*.cpp, false) \
    $$files($$PWD/src/infrastructure/repositories/*.cpp, false)

# Legacy core headers (preserved)
HEADERS += \
    CoreEntities.h \
    DispatchManager.h \
    SpatialGraph.h \
    mainwindow.h

# Automatic discovery of headers in the new 5-layer architecture
HEADERS += \
    $$files($$PWD/src/ui/*.h, false) \
    $$files($$PWD/src/ui/pages/*.h, false) \
    $$files($$PWD/src/ui/dialogs/*.h, false) \
    $$files($$PWD/src/domain/*.h, false) \
    $$files($$PWD/src/domain/entities/*.h, false) \
    $$files($$PWD/src/domain/enums/*.h, false) \
    $$files($$PWD/src/domain/policies/*.h, false) \
    $$files($$PWD/src/application/*.h, false) \
    $$files($$PWD/src/algorithms/*.h, false) \
    $$files($$PWD/src/infrastructure/*.h, false) \
    $$files($$PWD/src/infrastructure/repositories/*.h, false)

FORMS += \
    mainwindow.ui \
    $$files($$PWD/src/ui/*.ui, false) \
    $$files($$PWD/src/ui/pages/*.ui, false) \
    $$files($$PWD/src/ui/dialogs/*.ui, false)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target