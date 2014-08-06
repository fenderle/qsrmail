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

#ifndef QSRMAILMIMEMULTIPART_H
#define QSRMAILMIMEMULTIPART_H

#include "qsrmailabstractmimepart.h"
#include "qsrmailmimepart.h"

QT_BEGIN_NAMESPACE

class QSRMAILSHARED_EXPORT QsrMailMimeMultipart : public QsrMailAbstractMimePart
{
public:
    enum ContentType {
        MixedType, AlternativeType, DigestType, ParallelType
    };

public:
    QsrMailMimeMultipart();
    explicit QsrMailMimeMultipart(ContentType type);
    QsrMailMimeMultipart(const QsrMailMimeMultipart &other);
    QsrMailMimeMultipart &operator=(const QsrMailMimeMultipart &other);
    void swap(QsrMailMimeMultipart &other);
    ~QsrMailMimeMultipart();

    bool isNull() const;

    void setContentType(ContentType contentType);
    ContentType contentType() const;

    void setBoundary(const QByteArray &boundary);
    QByteArray boundary() const;

    void append(const QsrMailAbstractMimePart &part);
    QList<QsrMailAbstractPart> parts() const;
};

QT_END_NAMESPACE

#endif // QSRMAILMIMEMULTIPART_H
