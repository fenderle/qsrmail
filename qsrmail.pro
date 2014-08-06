# QsrMail - a SMTP client library for Qt
#
# Copyright 2014 Frank Enderle <frank.enderle@anamica.de>
#
# This file is part of QsrMail.
#
# QsrMail is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# QsrMail is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with QsrMail. If not, see <http://www.gnu.org/licenses/>.

QT += network
QT -= gui

TEMPLATE = lib

VERSION = 1.0.0

# build debug and release variants with different names
CONFIG += debug_and_release build_all

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

# set output directory

# handle static/dll builds
static {
    DESTDIR = $${PWD}/lib/static
    CONFIG += staticlib create_prl
} else {
    DESTDIR = $${PWD}/lib/dynamic
    CONFIG += dll
    win32:RC_FILE = qsrmail.rc
}

# enable library build mode
DEFINES += QSRMAIL_LIBRARY

INCLUDEPATH += include

OTHER_FILES += \
    qsrmail.pri \
    qsrmail.rc

HEADERS += \
    include/QsrMailAbstractEncoder \
    include/QsrMailAbstractMimePart \
    include/QsrMailAbstractPart \
    include/QsrMailAddress \
    include/QsrMailBase64Encoder \
    include/QsrMailBodyPart \
    include/QsrMailMessage \
    include/QsrMailMimeMultipart \
    include/QsrMailMimePart \
    include/QsrMailQpEncoder \
    include/QsrMailTransaction \
    include/QsrMailTransport \
    src/qsrmailabstractencoder.h \
    src/qsrmailabstractencoder_p.h \
    src/qsrmailabstractmimepart.h \
    src/qsrmailabstractpart.h \
    src/qsrmailabstractpart_p.h \
    src/qsrmailaddress.h \
    src/qsrmailaddress_p.h \
    src/qsrmailbase64encoder.h \
    src/qsrmailbase64encoder_p.h \
    src/qsrmailbodypart.h \
    src/qsrmailglobal.h \
    src/qsrmailheaders_p.h \
    src/qsrmailmessage.h \
    src/qsrmailmessage_p.h \
    src/qsrmailmimemultipart.h \
    src/qsrmailmimepart.h \
    src/qsrmailqpencoder.h \
    src/qsrmailqpencoder_p.h \
    src/qsrmailrenderer_p.h \
    src/qsrmailrfctools_p.h \
    src/qsrmailtransaction.h \
    src/qsrmailtransaction_p.h \
    src/qsrmailtransport.h \
    src/qsrmailtransport_p.h

SOURCES += \
    src/qsrmailabstractencoder.cpp \
    src/qsrmailabstractmimepart.cpp \
    src/qsrmailabstractpart.cpp \
    src/qsrmailaddress.cpp \
    src/qsrmailbase64encoder.cpp \
    src/qsrmailbodypart.cpp \
    src/qsrmailheaders.cpp \
    src/qsrmailmessage.cpp \
    src/qsrmailmimemultipart.cpp \
    src/qsrmailmimepart.cpp \
    src/qsrmailqpencoder.cpp \
    src/qsrmailrenderer.cpp \
    src/qsrmailrfctools.cpp \
    src/qsrmailtransaction.cpp \
    src/qsrmailtransport.cpp
