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

#ifndef QSRMAILBODYPART_H
#define QSRMAILBODYPART_H

#include "qsrmailabstractpart.h"

QT_BEGIN_NAMESPACE

class QIODevice;

class QSRMAILSHARED_EXPORT QsrMailBodyPart : public QsrMailAbstractPart
{
public:
    QsrMailBodyPart();
    QsrMailBodyPart(const QsrMailBodyPart &other);
    QsrMailBodyPart &operator=(const QsrMailBodyPart &other);
    void swap(QsrMailBodyPart &other);
    ~QsrMailBodyPart();

    bool isNull() const;

    void setAutoDelete(bool enabled);
    bool autoDelete() const;

    void setBody(const QByteArray &content);
    QByteArray body() const;

    void setBodyDevice(QIODevice *device);
    QIODevice *bodyDevice() const;

    static QsrMailBodyPart fromRawData(const QByteArray &data);
};

QT_END_NAMESPACE

#endif // QSRMAILBODYPART_H
