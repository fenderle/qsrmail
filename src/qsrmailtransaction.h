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

#ifndef QSRMAILTRANSACTION_H
#define QSRMAILTRANSACTION_H

#include "qsrmailglobal.h"
#include <QObject>
#include <QSslConfiguration>

#include "qsrmailtransport.h"

QT_BEGIN_NAMESPACE

class QsrMailMessage;
class QsrMailTransport;
class QsrMailError;

class QsrMailTransactionPrivate;
class QSRMAILSHARED_EXPORT QsrMailTransaction : public QObject
{
    Q_OBJECT
    Q_ENUMS(MailError)

public:
    enum TransactionError {
        NoError = 0,
        NoSenderError,
        NoRecipientsError,
        ResponseError,
        ConnectionError,
        TlsRequiredError,
        ResolverError,
        TimeoutError,
        AbortedError,
        DataError
    };

public:
    ~QsrMailTransaction();

    QsrMailTransport *transport() const;
    QsrMailMessage message() const;

    TransactionError error() const;
    QString errorText() const;
    int status() const;
    QString statusText() const;

    bool isEncrypted() const;
    QSslConfiguration sslConfiguration() const;

    bool isAuthenticated() const;
    QsrMailTransport::AuthMech authMech() const;
    QString username() const;

public Q_SLOTS:
    void abort();

Q_SIGNALS:
    void finished();    
    void error(QsrMailTransaction::TransactionError error);
    void progressUpdate(int percent);

private:
    explicit QsrMailTransaction(QsrMailTransport *transport);

private:
    Q_DECLARE_PRIVATE(QsrMailTransaction)
    friend class QsrMailTransportPrivate;

    QScopedPointer<QsrMailTransactionPrivate> d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QsrMailTransaction::TransactionError)

#endif // QSRMAILTRANSACTION_H
