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

#ifndef QSRMAILABSTRACTPART_H
#define QSRMAILABSTRACTPART_H

#include "qsrmailglobal.h"

#include <QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QsrMailBodyPart;
class QsrMailMimePart;
class QsrMailMimeMultipart;

class QsrMailAbstractPartPrivate;
class QSRMAILSHARED_EXPORT QsrMailAbstractPart
{
public:
    enum PartType {
        NullType, BodyPartType, MimePartType, MimeMultipartType
    };

public:
    QsrMailAbstractPart();
    QsrMailAbstractPart(const QsrMailAbstractPart &other);
    QsrMailAbstractPart &operator=(const QsrMailAbstractPart &other);
    void swap(QsrMailAbstractPart &other);
    virtual ~QsrMailAbstractPart();

    PartType type() const;
    bool canConvert(PartType conversion) const;

    QsrMailBodyPart toBodyPart() const;
    QsrMailMimePart toMimePart() const;
    QsrMailMimeMultipart toMimeMultipart() const;

    virtual bool isNull() const;
    bool isEmpty() const;

protected:
    explicit QsrMailAbstractPart(QsrMailAbstractPartPrivate *dd);

protected:
    QSharedDataPointer<QsrMailAbstractPartPrivate> d;
    friend class QsrMailRenderer;
};

QT_END_NAMESPACE

#endif // QSRMAILABSTRACTMIMEPART_H
