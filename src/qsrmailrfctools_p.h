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

#ifndef QSRMAILRFCTOOLS_P_H
#define QSRMAILRFCTOOLS_P_H

#include <QByteArray>
#include <QDateTime>

QT_BEGIN_NAMESPACE

class QsrMailRfcTools
{
public:
    static bool validateAddrSpec(const QByteArray &data);
    static bool validateDisplayName(const QByteArray &data);
    static QByteArray toEncodedWords(const QString &data);
    static QByteArray rfc2822Date(const QDateTime &dateTime);

private:
    static const char *skipCFWS(const char *c);
};

QT_END_NAMESPACE

#endif // QSRMAILRFCTOOLS_P_H
