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

#ifndef QSRMAIL_MAILMESSAGE_P_H
#define QSRMAIL_MAILMESSAGE_P_H

#include <QSharedData>
#include <QList>
#include <QDateTime>
#include <QString>
#include <QByteArray>
#include <QIODevice>

#include "qsrmailmessage.h"
#include "qsrmailheaders_p.h"
#include "qsrmailaddress.h"
#include "qsrmailmimemultipart.h"

QT_BEGIN_NAMESPACE

class QsrMailMessagePrivate : public QSharedData
{
public:
    QsrMailMessagePrivate();

    QsrMailHeaders cookHeaders() const;

public:
    QByteArray messageId;
    QsrMailHeaders headers;
    QsrMailAddress sender;
    QList<QsrMailAddress> from;
    QList<QsrMailAddress> to;
    QList<QsrMailAddress> replyTo;
    QList<QsrMailAddress> cc;
    QList<QsrMailAddress> bcc;
    QDateTime date;
    QString subject;
    QsrMailAbstractPart body;
};

QT_END_NAMESPACE

#endif // QSRMAIL_MAILMESSAGE_P_H
