# Note: You will have to copy etc folder to build folder

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += __USE_MINGW_ANSI_STDIO=1

QMAKE_CFLAGS += -Wno-unused-function
QMAKE_CFLAGS += -Wno-missing-braces
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-field-initializers

INCLUDEPATH += include
HEADERS += include/**
HEADERS += deps/*.h

SOURCES += src/main.c
SOURCES += src/module.c
SOURCES += deps/cglm.c
SOURCES += deps/flecs.c
SOURCES += deps/flecs_components_cglm.c
SOURCES += deps/flecs_components_geometry.c
SOURCES += deps/flecs_components_graphics.c
SOURCES += deps/flecs_components_gui.c
SOURCES += deps/flecs_components_input.c
SOURCES += deps/flecs_components_physics.c
SOURCES += deps/flecs_components_transform.c
SOURCES += deps/flecs_game.c
SOURCES += deps/flecs_systems_physics.c
SOURCES += deps/flecs_systems_sokol.c
SOURCES += deps/flecs_systems_transform.c



LIBS += -lws2_32 -lgdi32
