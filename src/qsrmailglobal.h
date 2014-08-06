/*****************************************************************************
** QsrMail - a SMTP client library for Qt
**
** Copyright 2014 Frank Enderle <frank.enderle@anamica.de>
**
** This file is part of QsrMail.
**
** QsrMail is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as
** published by the Free Software Foundation, either version 3 of
** the License, or (at your option) any later version.
**
** QsrMail is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with QsrMail. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef QSRMAILGLOBAL_H
#define QSRMAILGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifdef QSRMAIL_STATIC
#  define QSRMAILSHARED_EXPORT
#else
#  if defined(QSRMAIL_LIBRARY)
#    define QSRMAILSHARED_EXPORT Q_DECL_EXPORT
#  else
#    define QSRMAILSHARED_EXPORT Q_DECL_IMPORT
#  endif
#endif

#if defined(QSRMAIL_LIBRARY)
#define QSRMAIL_SHARED_NULL(type) \
    static QSharedDataPointer<type> &sharedNull() { \
        static QSharedDataPointer<type> *p=new QSharedDataPointer<type>(new type); \
        return *p; \
    }
#endif

/* taken from boost */
#if __cplusplus >= 201103L && defined(__has_warning)
#  if __has_feature(cxx_attributes) && __has_warning("-Wimplicit-fallthrough")
#    define QSRMAIL_FALLTHROUGH [[clang::fallthrough]]
#  endif
#endif

#ifndef QSRMAIL_FALLTHROUGH
#  define QSRMAIL_FALLTHROUGH do {} while(0)
#endif

#define QSRMAIL_VERSION_STR "1.0.0"
#define QSRMAIL_VERSION 0x010000

QT_END_NAMESPACE

#endif // QSRMAILGLOBAL_H
