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

#ifndef QSRMAILABSTRACTMIMEPART_H
#define QSRMAILABSTRACTMIMEPART_H

#include "qsrmailabstractpart.h"

#include <QDateTime>

QT_BEGIN_NAMESPACE

class QSRMAILSHARED_EXPORT QsrMailAbstractMimePart : public QsrMailAbstractPart
{
public:
    enum DispositionType {
        InlineDisposition, AttachmentDisposition
    };

public:
    QsrMailAbstractMimePart(const QsrMailAbstractMimePart &other);
    QsrMailAbstractMimePart &operator=(const QsrMailAbstractMimePart &other);
    void swap(QsrMailAbstractMimePart &other);
    ~QsrMailAbstractMimePart();

    void setRawHeader(const QByteArray &name, const QByteArray &value);
    void appendRawHeader(const QByteArray &name, const QByteArray &value);
    QByteArray rawHeader(const QByteArray &name) const;
    QList<QByteArray> rawHeaders(const QByteArray &name) const;

    void setContentType(const QByteArray &type);
    QByteArray contentType() const;

    void setContentId(const QByteArray &id);
    QByteArray contentId() const;

    void setContentEncoding(const QByteArray &encoding);
    QByteArray contentEncoding() const;

    void setContentDescription(const QString &description);
    QString contentDescription() const;

    void setContentDisposition(DispositionType type);
    DispositionType contentDisposition() const;

    void setFilename(const QString &name);
    QString filename() const;

    void setCreateDate(const QDateTime &date);
    QDateTime createDate() const;

    void setModificationDate(const QDateTime &date);
    QDateTime modificationDate() const;

    void setReadDate(const QDateTime &date);
    QDateTime readDate() const;

    void setSize(qint64 bytes);
    qint64 size() const;

protected:
    explicit QsrMailAbstractMimePart(QsrMailAbstractPartPrivate *dd);
};

QT_END_NAMESPACE

#endif // QSRMAILABSTRACTMIMEPART_H
