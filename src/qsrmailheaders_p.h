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

#ifndef QSRMAILHEADERS_P_H
#define QSRMAILHEADERS_P_H

#include <QList>
#include <QPair>
#include <QByteArray>

QT_BEGIN_NAMESPACE

class QsrMailHeaders
{
public:
    typedef QPair<QByteArray, QByteArray> HeaderPair;
    typedef QList<HeaderPair> HeaderList;

public:
    void setHeader(const QByteArray &name, const QByteArray &value);
    void appendHeader(const QByteArray &name, const QByteArray &value);
    void appendHeader(const QsrMailHeaders &other);
    HeaderList::ConstIterator findHeader(const QByteArray &name) const;
    bool hasHeader(const QByteArray &name) const;
    QByteArray value(const QByteArray &name) const;
    QList<QByteArray> values(const QByteArray &name) const;
    QByteArray renderHeaders() const;

public:
    HeaderList headers;
};

QT_END_NAMESPACE

#endif // QSRMAILHEADERS_P_H
