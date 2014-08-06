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

#ifndef QSRABSTRACTPART_P_H
#define QSRABSTRACTPART_P_H

#include <QSharedData>
#include <QIODevice>
#include <QDateTime>

#include "qsrmailheaders_p.h"

/* include headers of derivated classes to get access to all declarations */
#include "qsrmailbodypart.h"
#include "qsrmailmimepart.h"
#include "qsrmailmimemultipart.h"

QT_BEGIN_NAMESPACE

class QsrMailAbstractPartPrivate : public QSharedData
{
public:
    QsrMailAbstractPartPrivate(QsrMailAbstractPart::PartType type);
    QsrMailHeaders cookHeaders() const;

    bool isEmptyImpl() const;

    inline bool isBodyPart() const
    { return partType == QsrMailAbstractPart::BodyPartType; }

    inline bool isMimePart() const
    { return partType == QsrMailAbstractPart::MimePartType; }

    inline bool isMimeMultipart() const
    { return partType == QsrMailAbstractPart::MimeMultipartType; }

public:
    QsrMailAbstractPart::PartType partType;

    /* QsrMailBodyPart / QsrMailMimePart */
    QByteArray body;
    QIODevice *bodyDevice;
    bool autoDelete;

    /* QsrMailMimePart / QsrMailMimeMultipart */
    QsrMailHeaders headers;
    QByteArray contentType;
    QsrMailMimeMultipart::ContentType multipartContentType;
    QByteArray contentEncoding;
    QByteArray contentId;
    QString contentDescription;
    QsrMailAbstractMimePart::DispositionType dispositionType;
    QString filename;
    QDateTime createDate;
    QDateTime modificationDate;
    QDateTime readDate;
    qint64 size;

    /* QsrMailMimePart */
    QsrMailMimePart::Encoder encoder;

    /* QsrMailMimeMultipart */
    QByteArray boundary;
    QList<QsrMailAbstractPart> parts;
};

QT_END_NAMESPACE

#endif // QSRABSTRACTPART_P_H
