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

QSRMAIL_INCLUDE_PATH = $${PWD}/include
QSRMAIL_LIB = qsrmail
QSRMAIL_LIB_VERSION = 1

# add 'd' for debug versions
debug:QSRMAIL_LIB = $$join(QSRMAIL_LIB,,,d)

# seperate static and dynamic versions in different directories
# to force the linker selecting the right variant.
qsrmail_static {
    QSRMAIL_LIB_PATH = $${PWD}/lib/static
    DEFINES += QSRMAIL_STATIC
} else {
    QSRMAIL_LIB_PATH = $${PWD}/lib/dynamic
    win32:QSRMAIL_LIB = $$join(QSRMAIL_LIB,,,$${QSRMAIL_LIB_VERSION})
}

LIBS += -L$${QSRMAIL_LIB_PATH} -l$${QSRMAIL_LIB}

# generic
INCLUDEPATH += $${QSRMAIL_INCLUDE_PATH}
DEPENDPATH += $${QSRMAIL_INCLUDE_PATH}
