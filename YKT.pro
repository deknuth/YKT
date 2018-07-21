TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/main.c \
    src/socket.c \
    src/public.c \
    src/logic.c \
    src/aes.c \
    src/base64.c \
    src/hash.c \
    src/cjson.c \
    src/parse.c \
    src/pool.c \
    src/log.c \
    src/db.c \
    src/channel.c \
    src/hal.c \
    src/guard.c \
    src/times.c \
    src/mp3.c \
    src/offline.c

HEADERS += \
    inc/main.h \
    inc/aes.h \
    inc/logic.h \
    inc/base64.h \
    inc/hash.h \
    inc/cjson.h \
    inc/parse.h \
    inc/pool.h \
    inc/log.h \
    inc/db.h \
    inc/channel.h \
    inc/hal.h \
    inc/guard.h \
    inc/times.h \
    inc/mp3.h \
    inc/mad.h \
    inc/offline.h

