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

#ifndef QSRMAILTRANSPORT_H
#define QSRMAILTRANSPORT_H

#include "qsrmailglobal.h"
#include <QObject>
#include <QAbstractSocket>

QT_BEGIN_NAMESPACE

class QsrMailMessage;
class QsrMailTransaction;
class QsrMailTransportPrivate;
class QHostAddress;
class QSslConfiguration;

class QSRMAILSHARED_EXPORT QsrMailTransport : public QObject
{
    Q_OBJECT
    Q_ENUMS(TlsLevel)
    Q_ENUMS(AuthMech)

public:
    enum TlsLevel {
        TlsDisabled,
        TlsOptional,
        TlsRequired
    };

    enum AuthMech {
        DisabledMech,
        AutoSelectMech,
        CramMd5Mech,
        LoginMech,
        PlainMech
    };

public:
    explicit QsrMailTransport(QObject *parent = 0);
    ~QsrMailTransport();

    void setUser(const QString &username);
    QString user() const;

    void setPassword(const QString &passwd);
    QString password() const;

    void setAuthMech(AuthMech mechanism);
    AuthMech authMech() const;

    void setSystemIdentifier(const QByteArray &value);
    QByteArray systemIdentifier() const;

    void setTimeout(int timeout);
    int timeout() const;

    void setTlsLevel(QsrMailTransport::TlsLevel level);
    QsrMailTransport::TlsLevel tlsLevel() const;

    void setSslConfiguration(const QSslConfiguration &value);
    QSslConfiguration sslConfiguration() const;

    QsrMailTransaction *queueMessage(const QsrMailMessage &message);

    void sendMessages(const QString &serverHostname,
                      quint16 serverPort = 25,
                      QAbstractSocket::NetworkLayerProtocol
                      protocol = QAbstractSocket::AnyIPProtocol);

    void sendMessages(const QHostAddress &serverAddress,
                      quint16 serverPort = 25);

public Q_SLOTS:
    void abort();

Q_SIGNALS:
    void progressUpdate(int percent);
    void transactionFinished(QsrMailTransaction *transaction);
    void finished();

protected:
    QsrMailTransport(QsrMailTransportPrivate &qq, QObject *parent);

protected:
    Q_DECLARE_PRIVATE(QsrMailTransport)

    QScopedPointer<QsrMailTransportPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_stateChanged(QAbstractSocket::SocketState))
    Q_PRIVATE_SLOT(d_func(), void _q_socketReadyRead())
    Q_PRIVATE_SLOT(d_func(), void _q_encryptedBytesWritten(qint64))
    Q_PRIVATE_SLOT(d_func(), void _q_bytesWritten(qint64))
    Q_PRIVATE_SLOT(d_func(), void _q_writeMessageData())
    Q_PRIVATE_SLOT(d_func(), void _q_messageProgress(int, int))
    Q_PRIVATE_SLOT(d_func(), void _q_transactionFinished())
    Q_PRIVATE_SLOT(d_func(), void _q_processStates())
    Q_PRIVATE_SLOT(d_func(), void _q_timeout())
};

QT_END_NAMESPACE

#endif // QSRMAILTRANSPORT_H
