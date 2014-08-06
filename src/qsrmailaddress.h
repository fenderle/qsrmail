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

#ifndef QSRMAILMAILADDRESS_H
#define QSRMAILMAILADDRESS_H

#include "qsrmailglobal.h"
#include <QSharedDataPointer>
#include <QHash>
#include <QString>

QT_BEGIN_NAMESPACE

class QsrMailAddressPrivate;
class QSRMAILSHARED_EXPORT QsrMailAddress
{
public:
    QsrMailAddress();
    explicit QsrMailAddress(const QString &address);
    QsrMailAddress(const QsrMailAddress &other);
    QsrMailAddress &operator=(const QsrMailAddress &other);
    void swap(QsrMailAddress &other);
    virtual ~QsrMailAddress();

    QsrMailAddress(const QString &address, const QString &displayName);
    bool operator==(const QsrMailAddress &other) const;
    bool operator==(const QString &address) const;

    bool isNull() const;
    bool isValid() const;

    void setAddress(const QString &address);
    QString address() const;

    void setDisplayName(const QString &displayName);
    QString displayName() const;

    QByteArray toByteArray() const;
    QString toString() const;

private:
    QSharedDataPointer<QsrMailAddressPrivate> d;
};

inline uint qHash(const QsrMailAddress &key)
{ return qHash(key.address() + key.displayName()); }

QT_END_NAMESPACE

#endif // QSRMAILMAILADDRESS_H
