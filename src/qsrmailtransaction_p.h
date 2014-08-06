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

#ifndef QSRMAILTRANSACTION_P_H
#define QSRMAILTRANSACTION_P_H

#include "qsrmailmessage.h"
#include "qsrmailtransaction.h"
#include "qsrmailrenderer_p.h"

QT_BEGIN_NAMESPACE

class QsrMailTransaction;
class QsrMailTransactionPrivate
{
public:
    explicit QsrMailTransactionPrivate(QsrMailTransaction *qq);

    void finalize();

    void setError(QsrMailTransaction::TransactionError code,
                  const QString &text = QString());

    void setStatus(int code, const QList<QByteArray> &text);

    void setProgress(int percent);

    static QsrMailTransaction *createInstance(const QsrMailMessage &message,
                                              QsrMailTransport *transport);

public:
    Q_DECLARE_PUBLIC(QsrMailTransaction)

    QsrMailTransaction *q_ptr;
    QsrMailRenderer *renderer;
    QIODevice *device;

    QsrMailTransport *transport;
    QsrMailMessage message;
    QString messageId;
    QsrMailTransaction::TransactionError error;
    QString errorText;
    int status;
    QString statusText;

    bool encrypted;
    QSslConfiguration sslConfiguration;

    bool authenticated;
    QsrMailTransport::AuthMech authMech;
    QString username;
};

QT_END_NAMESPACE

#endif // QSRMAILTRANSACTION_P_H
