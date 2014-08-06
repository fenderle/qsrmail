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

#ifndef QSRMAIL_MAILMESSAGE_H
#define QSRMAIL_MAILMESSAGE_H

#include "qsrmailglobal.h"
#include <QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QDateTime;
class QIODevice;

class QsrMailAddress;
class QsrMailAbstractPart;

class QsrMailMessagePrivate;
class QSRMAILSHARED_EXPORT QsrMailMessage
{
public:
    QsrMailMessage();
    QsrMailMessage(const QsrMailMessage &other);
    QsrMailMessage &operator=(const QsrMailMessage &other);
    void swap(QsrMailMessage &other);
    virtual ~QsrMailMessage();

    bool isEmpty() const;

    void setMessageId(const QByteArray &messageId);
    QByteArray messageId() const;

    void setRawHeader(const QByteArray &name, const QByteArray &value);
    void appendRawHeader(const QByteArray &name, const QByteArray &value);
    QByteArray rawHeader(const QByteArray &name) const;
    QList<QByteArray> rawHeaders(const QByteArray &name) const;

    void setSender(const QsrMailAddress &senderAddress);
    QsrMailAddress sender() const;

    void setFrom(const QsrMailAddress &fromAddress);
    void setFrom(const QList<QsrMailAddress> &fromAddresses);
    void appendFrom(const QsrMailAddress &fromAddress);
    QList<QsrMailAddress> from() const;

    void setTo(const QsrMailAddress &toAddress);
    void setTo(const QList<QsrMailAddress> &toAddresses);
    void appendTo(const QsrMailAddress &toAddress);
    QList<QsrMailAddress> to() const;

    void setReplyTo(const QsrMailAddress &replyToAddress);
    void setReplyTo(const QList<QsrMailAddress> &replyToAddresses);
    void appendReplyTo(const QsrMailAddress &replyToAddress);
    QList<QsrMailAddress> replyTo() const;

    void setCc(const QsrMailAddress &ccAddress);
    void setCc(const QList<QsrMailAddress> &ccAddresses);
    void appendCc(const QsrMailAddress &ccAddress);
    QList<QsrMailAddress> cc() const;

    void setBcc(const QsrMailAddress &bccAddress);
    void setBcc(const QList<QsrMailAddress> &bccAddresses);
    void appendBcc(const QsrMailAddress &bccAddress);
    QList<QsrMailAddress> bcc() const;

    void setDate(const QDateTime &date);
    QDateTime date() const;

    void setSubject(const QString &subject);
    QString subject() const;

    void setBody(const QsrMailAbstractPart &part);
    QsrMailAbstractPart body() const;

private:
    QSharedDataPointer<QsrMailMessagePrivate> d;
    friend class QsrMailRenderer;
};

QT_END_NAMESPACE

#endif // QSRMAIL_MAILMESSAGE_H
