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

#ifndef QSRMAILTRANSPORT_P_H
#define QSRMAILTRANSPORT_P_H

#include "qsrmailtransport.h"
#include "qsrmailtransaction_p.h"

#include <QSslSocket>
#include <QQueue>
#include <QHostAddress>
#include <QTimer>

QT_BEGIN_NAMESPACE

class QsrMailTransport;
class QsrMailTransportPrivate
{
public:
    enum State {
        IdleState,
        ResolvingState,
        ResolvedState,
        ConnectingState,
        ConnectedState,
        BannerState,
        SessionInitState,
        TlsSetupState,
        EncryptedState,
        EncryptedSessionInitState,
        SessionSetupState,
        AuthState,
        ReadyToSendState,
        MailFromState,
        RcptToState,
        DataState,
        EndOfMessageState,
        DataSentState,
        ClosingState,
        DisconnectedState,
        FinishedState
    };

public:
    explicit QsrMailTransportPrivate(QsrMailTransport *qq);

    void _q_stateChanged(QAbstractSocket::SocketState socketState);
    void _q_socketReadyRead();
    void _q_writeMessageData();
    void _q_encryptedBytesWritten(qint64 size);
    void _q_bytesWritten(qint64 size);
    void _q_messageProgress(int processed, int total);
    void _q_transactionFinished();
    void _q_timeout();
    void _q_processStates();

private:
    void enumExtensions(const QList<QByteArray> &lines);

    QByteArray authResponse(const QString &challenge);
    QByteArray cramMd5Mech(const QByteArray &challenge,
                           const QByteArray &user,
                           const QByteArray &pass);
    QByteArray loginMech(const QByteArray &challenge,
                         const QByteArray &user,
                         const QByteArray &pass);
    QByteArray plainMech(const QByteArray &challenge,
                         const QByteArray &user,
                         const QByteArray &pass);

    QsrMailTransaction *queueMessageImpl(const QsrMailMessage &message);
    void sendMessagesImpl(State initState);
    bool setupTransaction();    

    void finalizeQueue(QsrMailTransaction::TransactionError error,
                       const QString &errorText);
    void finalizeQueue(QsrMailTransaction::TransactionError error);

    void write(const QByteArray &data);

public:
    /* Response helper */
    struct SmtpResponse
    {
        void reset();
        bool append(const QByteArray &data);

        QByteArray line;
        QList<QByteArray> lines;
        int code;
        bool isValid;
        bool isEmpty;
        bool isCompleted;
        bool isIntermediate;
        bool isTransientError;
        bool isPermanentError;
        bool isSuccess;
        bool isError;
    };

public:
    Q_DECLARE_PUBLIC(QsrMailTransport)

    /* instance data */
    QsrMailTransport *q_ptr;
    QTimer *timer;
    QSslSocket *socket;
    State state;
    bool interrupted;
    bool aborted;
    bool reachedRTS;    
    bool authenticated;
    int crlfState;

    /* SMTP extensions */
    bool hasStartTls;
    bool hasAuth;
    QsrMailTransport::AuthMech selectedAuthMech;

    /* transaction related data */
    SmtpResponse response;
    QQueue<QsrMailTransactionPrivate *> queue;
    int totalMessages;
    int processedMessages;
    QByteArray from;
    QSet<QByteArray> rcpts;
    QSet<QByteArray>::ConstIterator rcpt;
    QsrMailRenderer *waitingRenderer;

    /* member data */
    QString username;
    QString password;
    QsrMailTransport::AuthMech authMech;
    QByteArray systemIdentifier;
    int timeout;
    QString serverHostname;
    QAbstractSocket::NetworkLayerProtocol serverProtocol;
    QHostAddress serverAddress;
    quint16 serverPort;
    QsrMailTransport::TlsLevel tlsLevel;
};

QT_END_NAMESPACE

#endif // QSRMAILTRANSPORT_P_H
