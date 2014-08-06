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

#ifndef QSRMAILMIMEPART_H
#define QSRMAILMIMEPART_H

#include "qsrmailabstractmimepart.h"

QT_BEGIN_NAMESPACE

class QIODevice;
class QFileDevice;

class QSRMAILSHARED_EXPORT QsrMailMimePart : public QsrMailAbstractMimePart
{
public:
    enum Encoder {
        AutoDetectEncoder, PassthroughEncoder, QuotedPrintableEncoder,
        Base64Encoder
    };

public:
    QsrMailMimePart();
    QsrMailMimePart(const QsrMailMimePart &other);
    QsrMailMimePart &operator=(const QsrMailMimePart &other);
    void swap(QsrMailMimePart &other);
    ~QsrMailMimePart();

    bool isNull() const;

    void setAutoDelete(bool enabled);
    bool autoDelete() const;

    void setContentEncoder(Encoder encoder);
    Encoder contentEncoder() const;

    void setContentEncoding(const QByteArray &encoding);

    void setBody(const QByteArray &data);
    QByteArray body() const;

    void setBodyDevice(QIODevice *device);
    QIODevice *bodyDevice() const;

    static QsrMailMimePart fromFile(QFileDevice *file);
    static QsrMailMimePart fromRawData(const QString &filename,
                                       const QByteArray &data);
    static QsrMailMimePart fromDevice(const QString &filename,
                                      QIODevice *device);
    static QsrMailMimePart fromText(const QString &filename);
};

QT_END_NAMESPACE

#endif // QSRMAILMIMEPART_H
