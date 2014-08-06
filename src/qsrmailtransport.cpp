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

/*!
 * \class QsrMailTransport qsrmailtransport.h <QsrMailTransport>
 * \brief This class implements the SMTP transport and builts the heart of the
 * system.
 *
 * To send a message to the SMTP server the developer has to instanciate this
 * class and submit QsrMailMessage using the queueMessage() method. For every
 * QsrMailMessage the method returns a QsrMailTransaction object which enables
 * the developer to track the message's delivery progress and provides access
 * to several runtime properties like the Message-ID or error messages.
 *
 * Overall the system works similar as the QNetworkManager, QNetworkRequest,
 * QNetworkReply object of the Qt framework.
 *
 * Some of the features of this implementation of SMTP include:
 *
 * - RFC compliant implementation of the SMTP protocol
 * - TLS encryption using STARTTLS
 * - User authentication with support for PLAIN, LOGIN, CRAM-MD5
 * - Supports MIME extensions in QsrMailMessage
 *
 * The following code outlines the use of the class in a simple example:
 * \code
 * // create one (or more) message objects
 * QsrMailMessage msg;
 * msg.setFrom(QsrMailAddress("john.doe@foo.com"));
 * msg.setTo(QsrMailAddress("jane.doe@foo.com"));
 * msg.setSubject("Hello world!");
 * msg.setBody("My first message!");
 *
 * // instance the transport and deliver the message, connect progress
 * // indicator and delete the transport after delivery
 * QsrMailTransport *tsp = new QsrMailTransport();
 *
 * connect(tsp, &QsrMailTransport::progressUpdate, [=](int percent){
 *     qDebug() << percent << "% overall sent";
 * });
 *
 * connect(tsp, &QsrMailTransport::finished,
 *         tsp, &QsrMailTransport::deleteLater);
 *
 * // optionally set authentication credentials
 * tsp->setUser("john.doe@foo.com");
 * tsp->setPassword("some-secret-sauce");
 *
 * // queue the message(s), connect progress indicator and output
 * // the delivery result and cleanup after delivery completes
 * QsrMailTransaction *transaction = tsp->queueMessage(msg);
 *
 * connect(transaction, &QsrMailTransaction::progressUpdate, [=](int percent){
 *     qDebug() << percent << "% message sent";
 * });
 *
 * connect(transaction, &QsrMailTransaction::finished, [=](){
 *     qDebug() << "TRANSACTION_FINISHED";
 *     qDebug() << "messageId" << transaction->messageId();
 *     qDebug() << "error" << transaction->error();
 *     qDebug() << "errorText" << transaction->errorText();
 *     qDebug() << "status" << transaction->status();
 *     qDebug() << "statusText" << transaction->statusText();
 *     transaction->deleteLater();
 * });
 *
 * // start the actual delivery to the SMTP server
 * tsp->sendMessages("mail.server.foo");
 * \endcode
 */

/*!
 * \enum QsrMailTransport::TlsLevel
 * The encryption level required for communication with the SMTP
 * server.
 *
 * \var QsrMailTransport::TlsDisabled
 * No encryption is required and will not be used if available.
 *
 * \var QsrMailTransport::TlsOptional
 * Encryption will be used if the SMTP server provides it.
 *
 * \var QsrMailTransport::TlsRequired
 * Encryption is mandatory and the connection will fail if the SMTP server
 * does not provide encryption.
 */

/*!
 * \enum QsrMailTransport::AuthMech
 * The authentication mechanism to use for exchanging the credentials
 * with the server.
 *
 * \var QsrMailTransport::DisabledMech
 * Completely disable the authentication system. Even if the server advertises
 * authentiaction and *username* and *password* is set, the authentication will
 * not take place.
 *
 * \var QsrMailTransport::AutoSelectMech
 * Automatically select the authentication mechanism depending on the server
 * advertisment and the capabilities of the library.
 *
 * \var QsrMailTransport::CramMd5Mech
 * Use CRAM-MD5 authentication with the server. The security of this is 'ok'
 * since the password is not transfered in plaintext. However consider using
 * encrypted connections.
 *
 * \var QsrMailTransport::LoginMech
 * Use LOGIN authentication with the server. Insecure - no go without encrypted
 * connections.
 *
 * \var QsrMailTransport::PlainMech
 * Use PLAIN authentication with the server. Insecure - no go without encrypted
 * connections.
 */

/*!
 * \fn QsrMailTransport::progressUpdate(int percent)
 *
 * Track this signal to receive progress updates of the mail delivery. The
 * percentage is normalized to 100% over all messages in the queue. To get
 * the progress for a single message connect the progress signal of the
 * QsrMailTransaction object.
 *
 * \sa QsrMailTransaction::progressUpdate()
 */

/*!
 * \fn QsrMailTransport::transactionFinished(QsrMailTransaction *transaction)
 *
 * This signal is emitted when a particular message delivery has been completed.
 * The signal passes the related QsrMailTransaction object which can be
 * inspected for errors and so on. Eventually it is the developers responsibilty
 * to dispose the transaction using deleteLater().
 */

/*!
 * \fn QsrMailTransport::finished()
 *
 * Is emitted when all messages have been processed and the connection to the
 * server has been closed. This marks the end of the delivery process.
 */

/*!
 * \internal
 *
 * \class QsrMailTransportPrivate qsrmailtransport_p.h
 * \brief The implementation and private data class of the QsrMailTransport
 * class.
 *
 * The backbone of this implementation is a FSM which handles all the data
 * messaging and server communication. For better overview the state diagram
 * has been added here. However, the state diagram only reflects *the expected
 * workflow* without considering every possible error handled by the FSM which
 * would render the diagram unreadable.
 *
 * **Legend**
 *
 *  Color  | Usage
 *  ------ | ---------------------------------
 *  orange | Unencrypted traffic
 *  green  | Encrypted traffic
 *  blue   | Unencrypted or encrypted traffic
 *  red    | Error transistions
 *
 * \dot
 * digraph {
 *      node [shape="rectangle" fontname="sans" fontsize=9]
 *      edge [fontname="sans" fontsize=9]
 *
 *      "Idle" [shape="circle"];
 *      "Finished" [shape="circle"];
 *
 *      "Idle" -> "Resolving" [color="orange" label="Connect\nvia DNS"];
 *      "Idle" -> "Connecting" [color="orange" label="Connect\nvia QHostAddr"];
 *      "Resolving" -> "Resolved" [color="orange" label="DNS\nresolved"];
 *      "Resolved" -> "Disconnected" [color="crimson" label="Resolver\nfailed"];
 *      "Resolved" -> "Connecting" [color="orange" label="Connect\nresolved IP"];
 *      "Connecting" -> "Disconnected" [color="crimson" label="Can't\nconnect"];
 *      "Connecting" -> "Connected" [color="orange" label="Connection\nestablished"];
 *      "Connected" -> "Banner" [color="orange" label="Wait for\nserver banner"];
 *      "Banner" -> "SessionInit" [color="orange" label="EHLO"];
 *      "SessionInit" -> "SessionSetup" [color="orange" label="HELO\n(fallback)"]
 *      "SessionInit" -> "TlsSetup" [color="darkgreen" label="STARTTLS"];
 *      "SessionInit" -> "Closing" [color="crimson" label="QUIT\n(TLS required)"]
 *      "SessionInit" -> "SessionSetup" [color="orange" label="TLS not\nrequested"];
 *      "TlsSetup" -> "Encrypted" [color="darkgreen" label="SSL\nestablished"];
 *      "TlsSetup" -> "Disconnected" [color="crimson" label="SSL\nfailed"];
 *      "TlsSetup" -> "Closing" [color="crimson" label="QUIT\n(TLS rejected)"];
 *      "TlsSetup" -> "SessionSetup" [color="orange" label="Fallback to\nunencrypted"]
 *      "Encrypted" -> "EncryptedSessionInit" [color="darkgreen" label="EHLO\n(recheck)"];
 *      "EncryptedSessionInit" -> "SessionSetup" [color="darkgreen"];
 *      "SessionSetup" -> "Auth" [color="dodgerblue" label="AUTH\nselection"];
 *      "SessionSetup" -> "ReadyToSend" [color="dodgerblue" label="No authentication\nrequested"];
 *      "Auth" -> "ReadyToSend" [color="dodgerblue" label="Challenge\nResponse"];
 *      "ReadyToSend" -> "Closing" [color="dodgerblue" label="QUIT\n(queue empty)"]
 *      "ReadyToSend" -> "MailFrom" [color="dodgerblue" label="MAIL FROM"];
 *      "MailFrom" -> "RcptTo" [color="dodgerblue" label="RCPT TO"];
 *      "RcptTo" -> "RcptTo" [color="dodgerblue" label="RCPT TO\n(multiple)"];
 *      "RcptTo" -> "Data" [color="dodgerblue" label="DATA"];
 *      "Data" -> "EndOfMessage" [color="dodgerblue" label="Send message"];
 *      "EndOfMessage" -> "DataSent" [color="dodgerblue" label="Send EOM"];
 *      "EndOfMessage" -> "Disconnected" [color="crimson" label="Renderer\nerror"];
 *      "DataSent" -> "ReadyToSend" [color="dodgerblue" label="Finalize\nmessage"];
 *      "Closing" -> "Disconnected" [color="dodgerblue" label="Disconnect\nfrom host"];
 *      "Disconnected" -> "Connecting" [color="orange" label="Reconnect\n(More messages)"];
 *      "Disconnected" -> "Finished" [color="dodgerblue" label="Finalize\nqueue"];
 * }
 * \enddot
 */

/*!
 * \internal
 *
 * \enum QsrMailTransportPrivate::State
 * Represents the several states within the FSM which is used to send the
 * messages to the SMTP server. Thus the process of delivery **always** has
 * a valid state (or so the theory) representing the current step in the
 * process.
 *
 * The states usually get executed in the order top to bottom, but sometimes
 * also jump within the FSM, depending on the situation.
 *
 * \var QsrMailTransportPrivate::IdleState
 * The FSM is idle and has not yet been started.
 *
 * \var QsrMailTransportPrivate::ResolvingState
 * Start the resolver and lookup the serverHostname, respecting the selected
 * serverProtocol.
 *
 * \var QsrMailTransportPrivate::ResolvedState
 * The resolver has finished. Now check the result and start connecting the
 * server if the name has been resolved correctly to an IP address.
 *
 * \var QsrMailTransportPrivate::ConnectingState
 * The FSM is connects the server and waits for the connection to be
 * established.
 *
 * \var QsrMailTransportPrivate::ConnectedState
 * The server has been connected successfully and is now expected to send the
 * banner text.
 *
 * \var QsrMailTransportPrivate::BannerState
 * The banner has been received and the FSM sends the EHLO command.
 *
 * \var QsrMailTransportPrivate::SessionInitState
 * Response to the EHLO came back. If the server did not understand we retry
 * using the HELO command. Otherwise the FSM enumerates the SMTP extensions of
 * the server returned in the EHLO response; this is where the authentication
 * capabilities are assessed and the encryption is decided. If encryption is
 * possible and requested the STARTTLS command is issued which then continues
 * with the TlsSetupState. If no encryption is selected the FSM continues with
 * the SessionSetupState.
 *
 * \var QsrMailTransportPrivate::TlsSetupState
 * Upgrades the socket to encryption mode.
 *
 * \var QsrMailTransportPrivate::EncryptedState
 * The QSslSocket managed to establish an encrypted connection. This triggers
 * the EHLO command again to enumerate the extension in the encrypted
 * connection which might be different from unencrypted connections.
 *
 * \var QsrMailTransportPrivate::EncryptedSessionInitState
 * Enumerate the EHLO response while encrypted connections. Essentially this
 * is the same as SessionInitState for encrypted connections.
 *
 * \var QsrMailTransportPrivate::SessionSetupState
 * Session setup state occurs after enumeration of the SMTP extension and
 * possibly setting up the encryption. The next thing will be to start
 * authentication if it has been requested by the server. For authentication
 * the FSM sends the AUTH command indicating the selected authentication mech
 * and then enters the AuthState. If no authentication is selected the next
 * state is ReadyToSendState.
 *
 * \var QsrMailTransportPrivate::AuthState
 * The server replied to the AUTH command and sent a challenge which is
 * answered using the credentials supplied in username and password according
 * to the selected authentication mech.
 *
 * \var QsrMailTransportPrivate::ReadyToSendState
 * The FSM is now ready to actually send data. A long journey. In this state
 * the FSM issues the MAIL FROM command for the next message in the queue and
 * continues to the MailFromState. If the queue is empty the QUIT command is
 * issued to terminate the connection. The next state is then ClosingState.
 *
 * \var QsrMailTransportPrivate::MailFromState
 * The MAIL FROM command has been acknowledged by the server. In this state
 * the FSM issues the RCPT TO command for the first recipient of the current
 * message.
 *
 * \var QsrMailTransportPrivate::RcptToState
 * The RCPT TO command has been acknowledged. If the message contains more
 * recipient the FSM sends the next RCPT TO command and stays in this state.
 * When all receipients have been processed the next state is DataState.
 *
 * \var QsrMailTransportPrivate::DataState
 * This installs a QsrMailRenderer which renders the complete mail body and
 * puts it on the wire. The next state is EndOfMessageState after the
 * renderer is done with it's work.
 *
 * \var QsrMailTransportPrivate::EndOfMessageState
 * The message has been written to the server - now write the CRLF.CRLF
 * makrer to terminate the message.
 *
 * \var QsrMailTransportPrivate::DataSentState
 * The server acknowledged the data transfer with it's queue id. The next
 * state is ReadyToSendState and closes the circle.
 *
 * \var QsrMailTransportPrivate::ClosingState
 * The server acknowledged the QUIT command. The FSM issues the disconnection
 * of the host since the protocol has been shut down.
 *
 * \var QsrMailTransportPrivate::DisconnectedState
 * This state is entered when the socket detected a disconnection from the
 * server either willingly or unwillingly. The FSM then finalizes the remaining
 * messages in the queue and sets error states where required. The next state
 * is FinishedState.
 *
 * \var QsrMailTransportPrivate::FinishedState
 * This is the ending state and emits the finished() signal.
 */

/*!
 * \internal
 *
 * \class QsrMailTransportPrivate::SmtpResponse qsrmailtransport_p.h
 * \brief Helper to parse response lines from the SMTP server.
 */

/*!
 * \internal
 *
 * \var QsrMailTransportPrivate::SmtpResponse::line
 * The last valid line received from the server (without it's status code)
 *
 * \var QsrMailTransportPrivate::SmtpResponse::lines
 * All the lines of the response (sans status code)
 *
 * \var QsrMailTransportPrivate::SmtpResponse::code
 * The status code of the response. Valid after the first response line has
 * been parsed successfully.
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isValid
 * Set to true if the response is complete and valid.
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isEmpty
 * Set to true if the response contains any data (does not mean that it is
 * valid)
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isCompleted
 * Set to true when the response code class is 2xx
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isIntermediate
 * Set to true when the response code class is 3xx
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isTransientError
 * Set to true when the response code class is 4xx
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isPermanentError
 * Set to true when the response code class is 5xx
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isSuccess
 * Set to true when the response code is between 200 and 399
 *
 * \var QsrMailTransportPrivate::SmtpResponse::isError
 * Set to true when the response code is between 400 and 599
 */

#include "qsrmailtransport.h"
#include "qsrmailtransport_p.h"

#include "qsrmailmessage_p.h"

#include <QStringBuilder>
#include <QSslConfiguration>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QDnsLookup>
#include <QUuid>

QT_BEGIN_NAMESPACE

/*!
 * \internal
 *
 * Construct a data class for QsrMailTransportPrivate from *qq*.
 */
QsrMailTransportPrivate::QsrMailTransportPrivate(QsrMailTransport *qq) :
    q_ptr(qq),
    timer(0),
    socket(0),
    state(IdleState),
    interrupted(false),
    aborted(false),
    reachedRTS(false),
    authenticated(false),
    crlfState(0),
    hasStartTls(false),
    hasAuth(false),
    selectedAuthMech(QsrMailTransport::DisabledMech),
    totalMessages(0),
    processedMessages(0),
    waitingRenderer(0),
    authMech(QsrMailTransport::AutoSelectMech),
    systemIdentifier("localhost"),
    timeout(6000),
    serverProtocol(QAbstractSocket::AnyIPProtocol),
    serverPort(25),
    tlsLevel(QsrMailTransport::TlsOptional)
{
}

/*!
 * \internal
 *
 * This slot is called when the socket changes it's connection state.
 * For the Connected and UnconnectedState of the socket the handler triggers
 * the FSM states accordingly.
 */
void
QsrMailTransportPrivate::_q_stateChanged(QAbstractSocket::SocketState socketState)
{
    Q_Q(QsrMailTransport);

    if (socketState == QAbstractSocket::ConnectedState) {
        state = ConnectedState;
        QMetaObject::invokeMethod(q, "_q_processStates", Qt::QueuedConnection);
    } else if (socketState == QAbstractSocket::UnconnectedState) {
        state = DisconnectedState;
        QMetaObject::invokeMethod(q, "_q_processStates", Qt::QueuedConnection);
    }
}

/*!
 * \internal
 *
 * Reads the available (full) lines from the input, wraps them into
 * the SmtpResponse object and triggers the FSM for each complete server
 * response.
 */
void QsrMailTransportPrivate::_q_socketReadyRead()
{
    Q_Q(QsrMailTransport);

    while (socket->canReadLine()) {
        /* read the line and add it to the result object */
        QByteArray data(socket->readLine());
        response.append(data);

        if (response.isValid) {
            /* response is complete - run the FSM within the event */
            QMetaObject::invokeMethod(q, "_q_processStates",
                                      Qt::DirectConnection);

            /* clear response for the next server responses */
            response.reset();
        }
    }
}

/*!
 * \internal
 *
 * Is connected to the active renderer and called everytime the renderer
 * has data available. The data is consumed if the buffer has space available.
 * When the buffer is full *waitForBytesWritten* is set to enforce
 * _q_bytesWritten() to call this method when the socket flushed some data and
 * thus there is now space in the buffer. This is not very elegant, but
 * sufficient.
 */
void QsrMailTransportPrivate::_q_writeMessageData()
{
    Q_Q(QsrMailTransport);

    /* Since the function is not only called by the renderer's signal
     * we cannot use the sender as a fool proof version of getting the
     * associated renderer object. However if we are called from the
     * bytesWritten() slot we previously set the waitingRenderer member
     * to the renderer which has been active before, so we can use that
     * information. It is not sufficient to just take the head of the
     * queue and pull the renderer since we cannot be sure that this
     * is not some old signal just arriving, while we already process the
     * next message (at least in theory). So to be really sure we compare
     * the renderer address of the queue head with the member data. If they
     * differ something is rellay not right and we just do nothing.
     */
    QsrMailRenderer *r = waitingRenderer;
    waitingRenderer = 0;

    if (r != 0 && (queue.isEmpty() || r != queue.head()->renderer))
        return;
    else if (r == 0)
        r = qobject_cast<QsrMailRenderer *>(q->sender());

    /* read all available chunks and put them to the device */
    forever {
        /* don't flood the socket buffer */
        int maxSize = r->bufferSize();

        if (socket->isEncrypted())
            maxSize -= socket->encryptedBytesToWrite();
        else
            maxSize -= socket->bytesToWrite();

        if (maxSize <= 0) {
            /* we need to wait for data to be written to the socket */
            waitingRenderer = r;
            break;
        }

        /* check for data */
        int size = r->bytesAvailable();
        if (size <= 0)
            break;

        /* make sure maxSize is not exceeded */
        if (size > maxSize)
            size = maxSize;

        /* detect CRLF at end of buffer */
        const char *data = r->dataPointer();
        if (size == 1) {
            if (crlfState == 1 && *data == 10)
                crlfState = 2;
            else if (*data == 13)
                crlfState = 1;
            else
                crlfState = 0;
        } else {
            if (*(data+size-2) == 13 && *(data+size-1) == 10)
                crlfState = 2;
            else if (*(data+size-1) == 13)
                crlfState = 1;
            else
                crlfState = 0;
        }

        socket->write(data, size);
        r->advanceDataPointer(size);
    }
}

/*!
 * \internal
 *
 * Triggered when the socket has data written and freed up space in the buffer.
 * If *waitForBytesWritten* is set this method calls _q_writeMessageData() to
 * acquire more data in the buffer.
 *
 * This also triggers the timer since data should be written pretty reliable.
 */
void QsrMailTransportPrivate::_q_encryptedBytesWritten(qint64 size)
{
    Q_UNUSED(size)

    /* retrigger the timer to avoid timeouts */
    if (timer->isActive())
        timer->start();

    /* call writeMessageData if we have a renderer waiting to free up space */
    if (waitingRenderer != 0)
        _q_writeMessageData();
}

/*!
 * \internal
 *
 * Like _q_encryptedBytesWritten() but handles plain sockets. We cannot just
 * use only the bytesWritten() signal since this is handled differently
 * depending on the encryption of the transfer. If the data is encrypted then
 * bytesWritten() is emitted when the socket writes to its buffers for
 * encryption. This is not real traffic (it is buffered). For encrypted sockets
 * the encryptedBytesWritten() signal is the one to go with. However if the
 * socket is not encrypted we have to use the normal bytesWritten() signal.
 * Now to avoid messing up our signals and slots through useless reconnects
 * we connect both signals and decide while runtime what to do or not to do.
 */
void QsrMailTransportPrivate::_q_bytesWritten(qint64 size)
{
    /* if the socket is plaintext we need to relay the signal call */
    if (!socket->isEncrypted())
        _q_encryptedBytesWritten(size);
}

/*!
 * \internal
 *
 * Is connected to the active renderer and is triggered everytime the renderer
 * processed a chunk of data (that is for example a mime part or headers, etc).
 *
 * The method triggers first the QsrMailTransaction::progressUpdate() event
 * to reflect the current messages progress and then triggers
 * QsrMailTransport::progressUpdate() to reflect the overall progress of
 * delivery.
 */
void QsrMailTransportPrivate::_q_messageProgress(int processed, int total)
{
    Q_Q(QsrMailTransport);

    /* calculate the percentage; make sure it does not exceed 100% */
    int percent = processed * 100 / total;
    if (percent > 100)
        percent = 100;

    /* update transaction's progress */
    queue.head()->setProgress(percent);

    /* update overall progress to reflect the current transaction */
    percent = (processedMessages * 100 + percent) / totalMessages;
    emit q->progressUpdate(percent > 100 ? 100 : percent);
}

/*!
 * \internal
 *
 * Is connected to every transaction and ensures that, when the transaction
 * is finished, the transactionFinished() signal fires, followed by
 * progressUpdate() to reflect the completed message.
 */
void QsrMailTransportPrivate::_q_transactionFinished()
{
    Q_Q(QsrMailTransport);

    /* fetch the transaction which trigger the event */
    QsrMailTransaction *t = qobject_cast<QsrMailTransaction *>(q->sender());
    const QsrMailTransactionPrivate *p = t->d_func();

    /* disconnect signals */
    p->renderer->disconnect(q);
    t->disconnect(q);

    /* abort renderer */
    p->renderer->abort();

    /* relay the event */
    emit q->transactionFinished(t);

    /* update overall progress to reflect the finished transaction */
    int percent = ++processedMessages * 100 / totalMessages;
    emit q->progressUpdate(percent > 100 ? 100 : percent);
}

/*!
 * \internal
 *
 * Is triggered when the timeout timer fires. This sets the *interrupte*
 * flag and immediately disconnects the server, which will trigger a
 * statechange on the socket which in turn will trigger the FSM
 * DisconnectedState which then tears everything appart and finalizes all
 * messages.
 */
void QsrMailTransportPrivate::_q_timeout()
{
    /* disconnect the socket connection - this will trigger the
     * UnconnectedState on the socket.
     */
    interrupted = true;
    socket->disconnectFromHost();
}

/*!
 * \internal
 *
 * The FSM processor handles all states of the FSM. For a better overview of
 * this complex state machine see the class documentation.
 *
 * This method also triggers the timer to avoid timeouts since the FSM is
 * regulary called.
 *
 * \sa QsrMailTransportPrivate
 */
void QsrMailTransportPrivate::_q_processStates()
{
    Q_Q(QsrMailTransport);
    int code = response.code;

    /* Trigger timer so we do not timeout */
    if (timer->isActive())
        timer->start();

    /* The state machine loops until a return statement is executed.
     * The explicit use of return and continue statements is intented
     * to make the behaviour more clear.
     */
    forever {
        if (state == ResolvingState) {
            /* Resolve the hostname into a protocol and an address */
            QDnsLookup::Type type = QDnsLookup::ANY;

            switch (serverProtocol) {
            case QAbstractSocket::IPv4Protocol:
                type = QDnsLookup::A;
                break;

            case QAbstractSocket::IPv6Protocol:
                type = QDnsLookup::AAAA;
                break;

            case QAbstractSocket::AnyIPProtocol:
            case QAbstractSocket::UnknownNetworkLayerProtocol:
                break;
            }

            QDnsLookup *resolver = new QDnsLookup(type, serverHostname, q);
            q->connect(resolver, SIGNAL(finished()),
                       q, SLOT(_q_processStates()));
            q->connect(resolver, SIGNAL(finished()),
                       resolver, SLOT(deleteLater()));

            /*
             * The resolver finished slot will trigger the fsm again, after
             * which the next state will be ResolvedState
             */
            state = ResolvedState;
            resolver->lookup();
            return;
        } else if (state == ResolvedState) {
            /* Resolver returned a result - cast the event object */
            QDnsLookup *resolver = qobject_cast<QDnsLookup *>(q->sender());
            Q_ASSERT(resolver != 0);

            /* On error flush the queue */
            if (resolver->error() != QDnsLookup::NoError) {
                finalizeQueue(QsrMailTransaction::ResolverError,
                              resolver->errorString());
                state = IdleState;
                return;
            }

            /* Select a random address for connection */
            const QList<QDnsHostAddressRecord> records(resolver->hostAddressRecords());
            serverAddress = records.at(qrand() % records.size()).value();
            state = ConnectingState;
            continue;
        } else if (state == ConnectingState) {
            /* Connect to the server stored in serverAddress */
            socket->connectToHost(serverAddress, serverPort);

            /* The state variable will be advanced by _q_stateChanged() */
            return;
        } else if (state == ConnectedState) {
            /* Connection is established; reset states and wait for
             * the banner
             */
            authenticated = false;

            state = BannerState;
            return;
        } else if (state == BannerState && code == 220) {
            /* 220: service ready */
            write("EHLO " % systemIdentifier);
            state = SessionInitState;
            return;
        } else if (state == SessionInitState && code > 499 && code < 510) {
            /* EHLO failed with some invalid command error; retry with HELO */
            write("HELO " % systemIdentifier);
            state = SessionSetupState;
            return;
        } else if (state == SessionInitState && code == 250) {
            /* Enumerate the extension */
            enumExtensions(response.lines);

            /* Establish TLS, if required */
            if (tlsLevel == QsrMailTransport::TlsOptional && hasStartTls) {
                /* Server provides TLS and we MAY use it */
                write("STARTTLS");
                state = TlsSetupState;
                return;
            } else if (tlsLevel == QsrMailTransport::TlsRequired) {
                if (hasStartTls) {
                    /* Server provides TLS and we MUST use it */
                    write("STARTTLS");
                    state = TlsSetupState;
                    return;
                } else {
                    /* Server has no tls but we REQUIRE it */
                    finalizeQueue(QsrMailTransaction::TlsRequiredError);
                    write("QUIT");
                    state = ClosingState;
                    return;
                }
            }

            /* No TLS connection - jump directly to SessionSetupState */
            state = SessionSetupState;
            continue;
        } else if (state == TlsSetupState && code == 220) {
            /* Encryption will advance the state */
            q->connect(socket, SIGNAL(encrypted()),
                       q, SLOT(_q_processStates()));

            /* Server agreed to TLS so try to set it up */
            socket->startClientEncryption();

            /* Next is encrypted - failure wil raise ssl error */
            state = EncryptedState;
            return;
        } else if (state == TlsSetupState && code == 454) {
            /* TLS not available */
            if (tlsLevel == QsrMailTransport::TlsRequired) {
                /* TLS is required - tear this connection down */
                finalizeQueue(QsrMailTransaction::TlsRequiredError);
                write("QUIT");
                state = ClosingState;
                return;
            }

            /* Fallback to unencrypted if encryption is not mandatory */
            state = SessionSetupState;
            continue;
        } else if (state == EncryptedState) {
            /* Disconnect signal after encryption is established */
            q->disconnect(socket, SIGNAL(encrypted()),
                          q, SLOT(_q_processStates()));

            /* RFC2487 section 4.2 requires to resend EHLO and enumerate the
             * response again (public/private EHLO responses).
             */
            write("EHLO " % systemIdentifier);
            state = EncryptedSessionInitState;
            return;
        } else if ((state == EncryptedSessionInitState && code == 250) ||
                   (state == SessionSetupState && code == 250)) {
            /* Enum extensions again as there was an EHLO after STARTTLS */
            if (state == EncryptedSessionInitState)
                enumExtensions(response.lines);

            /* Try authentication... */
            if (selectedAuthMech != QsrMailTransport::DisabledMech
                    && (!username.isEmpty() || !password.isEmpty())) {
                if (selectedAuthMech == QsrMailTransport::CramMd5Mech)
                    write("AUTH CRAM-MD5");
                else if (selectedAuthMech == QsrMailTransport::LoginMech)
                    write("AUTH LOGIN");
                else if (selectedAuthMech == QsrMailTransport::PlainMech)
                    write("AUTH PLAIN");

                /* Enter authentication state */
                state = AuthState;
                return;
            }

            /* Can't setup AUTH - try without */
            state = ReadyToSendState;
            continue;
        } else if (state == AuthState && code == 334) {
            /* The server sent a challenge - generate the response */
            write(authResponse(response.line));
            return;
        } else if (state == AuthState && code == 235) {
            /* AUTH successfull - continue with the messages */
            authenticated = true;
            state = ReadyToSendState;
            continue;
        } else if (state == ReadyToSendState) {
            /* We reached the ReadyToSendState so the connection is valid
             * and we can try to reestablish it if we have to disconnect
             * or get disconnected.
             */
            reachedRTS = true;

            /* Try to setup a transaction */
            while (!queue.isEmpty()) {
                if (setupTransaction()) {
                    /* Start the SMTP dialog */
                    write("MAIL FROM:<" % from % ">");
                    state = MailFromState;
                    return;
                }
            }

            /* No more transactions left */
            write("QUIT");
            state = ClosingState;
            return;
        } else if ((state == MailFromState && code == 250) ||
                   (state == RcptToState && code == 250)) {
            /* Send all recipients to the server */
            if (rcpt != rcpts.constEnd()) {
                write("RCPT TO:<" % *rcpt++ % ">");
                state = RcptToState;
                return;
            }

            /* Init DataState */
            write("DATA");
            state = DataState;
            return;
        } else if (state == DataState && code == 354) {
            /* Init and start renderer stage */
            waitingRenderer = 0;
            QsrMailRenderer *r = queue.head()->renderer;
            q->connect(r, SIGNAL(readChannelFinished()),
                       q, SLOT(_q_processStates()));
            q->connect(r, SIGNAL(error()),
                       q, SLOT(_q_processStates()));
            q->connect(r, SIGNAL(readyRead()),
                       q, SLOT(_q_writeMessageData()));
            q->connect(r, SIGNAL(progressUpdate(int, int)),
                       q, SLOT(_q_messageProgress(int, int)));
            r->renderMessage();

            state = EndOfMessageState;
            return;
        } else if (state == EndOfMessageState) {
            /* Check for renderer error */
            QsrMailRenderer *r = queue.head()->renderer;
            if (!r->lastError().isEmpty()) {
                /* Finalize transaction with error */
                QsrMailTransactionPrivate *t = queue.dequeue();
                t->setError(QsrMailTransaction::DataError,
                            r->lastError());
                t->finalize();

                /* The message was broken, but we have no possibility
                 * to interrupt the protocol; the best we can do is
                 * to drop the connection.
                 */
                socket->disconnectFromHost();
                return;
            }

            /* Write end-of-message, prepend CRLF if required */
            if (crlfState != 2)
                socket->write("\r\n");
            socket->write(".\r\n");

            state = DataSentState;
            return;
        } else if (state == DataSentState && code == 250) {
            /* Finalize transaction */
            QsrMailTransactionPrivate *t = queue.dequeue();
            t->setError(QsrMailTransaction::NoError);
            t->setStatus(code, response.lines);
            t->finalize();

            /* Prepare next message */
            state = ReadyToSendState;
            continue;
        } else if (state == ClosingState) {
            /* Connection is shutting down or a timeout occured... */
            socket->disconnectFromHost();
            return;
        } else if (state == DisconnectedState) {
            /* The socket got disconnected. Either we did this to ourselve
             * or it was dropped by the server. Our plan is to reestablish
             * the connection if we had once reached the ReadyToSend state
             * and if the queue is not empty. If we didn't reach the RTS State
             * the queue will be flushed with a connection error.
             */
            if (!queue.isEmpty()) {
                if (reachedRTS) {
                    /* We saw RTS at least once - so retry the connect */
                    timer->start();
                    state = ConnectingState;
                    continue;
                }

                /* Make sure the timer has stopped and is not messing around */
                timer->stop();

                /* Check cause of the error and abort the messages
                 * accordingly
                 */
                if (interrupted) {
                    finalizeQueue(QsrMailTransaction::TimeoutError);
                } else if (aborted) {
                    finalizeQueue(QsrMailTransaction::AbortedError);
                } else {
                    /* error due to some socket or ssl problem */
                    finalizeQueue(QsrMailTransaction::ConnectionError,
                                  socket->errorString());
                }
            }

            /* This is it - everything comes to an end */
            state = FinishedState;
            continue;
        } else if (state == FinishedState) {
            /* FSM is complete */
            emit q->finished();
            return;
        } else {
            /* If the error occured during sending of a particular message
             * reject that message with the server response and reset the
             * protocol - then proceed with the next message.
             */
            if (state >= MailFromState && state <= DataSentState) {
                QsrMailTransactionPrivate *t = queue.dequeue();
                t->setStatus(code, response.lines);
                t->setError(QsrMailTransaction::ResponseError);
                t->finalize();

                write("RSET");
                state = ReadyToSendState;
                return;
            }

            /* This is an unrecoverable protocol error so cancel all
             * messages left in the queue with the server response and
             * gracefully close the connection.
             */
            finalizeQueue(QsrMailTransaction::ResponseError);
            write("QUIT");
            state = ClosingState;
            return;
        }
    }
}

/*!
 * \internal
 *
 * Enumerate the SMTP extensions available in *lines*. The *lines* array
 * is typically directly obtained from the response object of an EHLO
 * reply. Only extensions are enumerated which are actually used within
 * the FSM. In particular this is:
 *
 * - STARTTLS which sets the *hasStartTls* flag
 * - AUTH which sets the *hasAuth* flag to indicate the server offers
 *   authentication and also selects the auth mech for authentication
 *   based on the servers advertisment.
 */
void QsrMailTransportPrivate::enumExtensions(const QList<QByteArray> &lines)
{
    /* reset the states */
    hasStartTls = false;
    hasAuth = false;
    selectedAuthMech = QsrMailTransport::DisabledMech;

    /* enum states from response lines */
    for (int i=0, size=lines.size(); i<size; i++) {
        QList<QByteArray> parts(lines[i].split(' '));
        if (parts.isEmpty())
            continue;

        if (parts[0] == "STARTTLS") {
            /* server has STARTTLS extension */
            hasStartTls = true;
        } else if (parts[0] == "AUTH") {
            hasAuth = true;

            /* select authentication mech */
            if (authMech == QsrMailTransport::AutoSelectMech) {
                /* server supports SMTP-AUTH */
                if (parts.contains("CRAM-MD5"))
                    selectedAuthMech = QsrMailTransport::CramMd5Mech;
                else if (parts.contains("LOGIN"))
                    selectedAuthMech = QsrMailTransport::LoginMech;
                else if (parts.contains("PLAIN"))
                    selectedAuthMech = QsrMailTransport::PlainMech;
                else
                    selectedAuthMech = QsrMailTransport::DisabledMech;
            } else {
                selectedAuthMech = authMech;
            }
        }
    }
}

/*!
 * \internal
 *
 * Return an authentication response for a *challange*. The response is
 * dependend on the selected authentication mech. This method delegates
 * the challenge to the appropriate handler or just returns an null
 * QByteArray if an auth mech is not selected.
 */
QByteArray QsrMailTransportPrivate::authResponse(const QString &challenge)
{
    QByteArray user(username.toUtf8());
    QByteArray pass(password.toUtf8());

    switch (selectedAuthMech) {
    case QsrMailTransport::CramMd5Mech:
        return cramMd5Mech(challenge.toLatin1(), user, pass);

    case QsrMailTransport::LoginMech:
        return loginMech(challenge.toLatin1(), user, pass);

    case QsrMailTransport::PlainMech:
        return plainMech(challenge.toLatin1(), user, pass);

    case QsrMailTransport::AutoSelectMech:
    case QsrMailTransport::DisabledMech:
        break;
    }

    return QByteArray();
}

/*!
 * \internal
 *
 * Calculate a CRAM-MD5 response based on *challange*, *user* and *pass*.
 * The implementation is described in RFC2195 and RFC2104. This is pretty
 * secure as the password is not transmitted in plaintext.
 */
QByteArray QsrMailTransportPrivate::cramMd5Mech(const QByteArray &challenge,
                                                const QByteArray &user,
                                                const QByteArray &pass)
{
    /* RFC2195, RFC2104 */
    /* if the secret exceeds 64 byte we should use the md5 as input */
    QByteArray secret(pass);
    if (secret.size() > 64)
        secret = QCryptographicHash::hash(secret, QCryptographicHash::Md5);

    /* this is the text we should key */
    QByteArray text(QByteArray::fromBase64(challenge));

    /* H(K XOR opad, H(K XOR ipad, text))
     * H    - md5
     * K    - secret, padded to 64 byte
     * opad - 0x5c * 64
     * ipad - 0x36 * 64
     */
    QByteArray padded(secret.append(QByteArray(64, 0)).left(64));
    QByteArray ipad(padded);
    for (int i=0; i<64; i++) ipad[i] = ipad[i] ^ 0x36;
    QByteArray opad(padded);
    for (int i=0; i<64; i++) opad[i] = opad[i] ^ 0x5c;

    ipad.append(text);
    opad.append(QCryptographicHash::hash(ipad, QCryptographicHash::Md5));

    /* build hash */
    QByteArray result(QCryptographicHash::hash(opad, QCryptographicHash::Md5)
                      .toHex().toLower());

    /* prepend with username and turn to base64 */
    return QByteArray(user % " " % result).toBase64();
}

/*!
 * \internal
 *
 * Calculate the LOGIN response based on *challenge*, *user* and *pass*.
 * The function decodes the Base64 'challenge' which is actually in plaintext
 * 'Username:' or 'Password:' and returns the Base64 encoded value for the
 * requested field. This is obviously insecure since the data is only a bit
 * obfuscated. Use this only on SSL enabled sockets.
 */
QByteArray QsrMailTransportPrivate::loginMech(const QByteArray &challenge,
                                              const QByteArray &user,
                                              const QByteArray &pass)
{
    QByteArray text(QByteArray::fromBase64(challenge));

    if (text == "Username:")
        return user.toBase64();
    if (text == "Password:")
        return pass.toBase64();

    return QByteArray();
}

/*!
 * \internal
 *
 * Calculate the PLAIN response based on *user* and *pass*. The *challenge*
 * is not really used within this method and is almost certainly just empty.
 * According to RFC4616 the PLAIN response is '\0username\0password' Base64
 * encoded - again not secure either. Use this only on SSL enabled sockets.
 */
QByteArray QsrMailTransportPrivate::plainMech(const QByteArray &challenge,
                                              const QByteArray &user,
                                              const QByteArray &pass)
{
    Q_UNUSED(challenge);

    /* RFC4616 */
    return QByteArray('\0' % user.left(255)
                      % '\0' % pass.left(255)).toBase64();
}

/*!
 * \internal
 *
 * Implementation of QsrMailTransport::queueMessage(). Implementation is
 * separated from frontend due to access restrictions on QsrMailTransaction.
 */
QsrMailTransaction *
QsrMailTransportPrivate::queueMessageImpl(const QsrMailMessage &message)
{
    Q_Q(QsrMailTransport);

    QsrMailTransaction *t = QsrMailTransactionPrivate::createInstance(message, q);
    QsrMailTransactionPrivate *p = t->d_func();

    /* connect transaction itself for signal relaying and cleanup */
    QObject::connect(t, SIGNAL(finished()), q, SLOT(_q_transactionFinished()));

    /* tracking the private data pointer is sufficient - we can always
     * use q_func() to access the interface
     */
    queue.enqueue(p);
    return t;
}

/*!
 * \internal
 *
 * Initialize the FSM states and start the state machine.
 *
 * Sets the initial state to *initState* and begins execution. The caller
 * has to ensure that the server connection details are set according to
 * the *initState*. *initState* must be on of the two values:
 *
 * - **QsrMailTransportPrivate::ResolvingState**
 *   If the server address is supplied via serverHostname and thus needs
 *   resolution.
 * - **QsrMailTransportPrivate::ConnectingState**
 *   If the server address is supplied via serverAddress
 *
 * If the queue contains no messages this method does nothing and directly
 * emits the finished signal.
 *
 * \sa QsrMailTransportPrivate
 */
void QsrMailTransportPrivate::sendMessagesImpl(State initState)
{
    Q_Q(QsrMailTransport);

    if (queue.isEmpty()) {
        emit q->finished();
        return;
    }

    state = initState;
    totalMessages = queue.size();
    processedMessages = 0;
    response.reset();
    interrupted = false;
    aborted = false;
    reachedRTS = false;
    authenticated = false;
    crlfState = 0;

    timer->start(timeout);

    QMetaObject::invokeMethod(q, "_q_processStates", Qt::QueuedConnection);
}

/*!
 * \internal
 *
 * Setup the current transaction for sending. This includes setting up the
 * member data for the FSM and checking for mandatory data. The method
 * returns false if the message does not meet the required criterias.
 */
bool QsrMailTransportPrivate::setupTransaction()
{
    /* get head transaction */
    QsrMailTransactionPrivate *t = queue.head();
    const QsrMailMessage &msg = t->message;

    /* setup transport info data */
    t->encrypted = socket->isEncrypted();
    t->sslConfiguration = socket->sslConfiguration();
    t->authenticated = authenticated;
    t->authMech = selectedAuthMech;
    t->username = username;

    /* setup sender data */
    from.clear();
    if (msg.sender().isValid())
        from = msg.sender().toByteArray();
    else if (!msg.from().isEmpty())
        from = msg.from().first().toByteArray();

    /* setup recipients */
    QList<QsrMailAddress> recipients;
    recipients.append(msg.to());
    recipients.append(msg.cc());
    recipients.append(msg.bcc());

    rcpts.clear();
    for (int i=0, size=recipients.size(); i<size; ++i)
        rcpts.insert(recipients.at(i).toByteArray());
    rcpt = rcpts.constBegin();

    /* message preflight check */
    if (from.isEmpty()) {        
        queue.dequeue();
        t->setError(QsrMailTransaction::NoSenderError);
        t->finalize();
        return false;
    }

    if (rcpts.isEmpty()) {
        queue.dequeue();
        t->setError(QsrMailTransaction::NoRecipientsError);
        t->finalize();
        return false;
    }

    return true;
}

/*!
 * \internal
 *
 * Finalize all messages in the queue. Sets *error* and *errorText* for
 * all the messages in the queue. Also aborts the current renderer if any.
 */
void QsrMailTransportPrivate::finalizeQueue(
        QsrMailTransaction::TransactionError error, const QString &errorText)
{
    /* finalize the rest of the queue */
    while (!queue.isEmpty()) {
        QsrMailTransactionPrivate *t = queue.dequeue();

        if (response.isValid)
            t->setStatus(response.code, response.lines);

        t->setError(error, errorText);
        t->finalize();
    }
}

/*!
 * \internal
 *
 * \overload
 */
void QsrMailTransportPrivate::finalizeQueue(
        QsrMailTransaction::TransactionError error)
{
    finalizeQueue(error, QString());
}

/*!
 * \internal
 *
 * Write command data to the socket. This is used to write the protocol
 * commands to the server.
 */
void QsrMailTransportPrivate::write(const QByteArray &data)
{
    socket->write(data % "\r\n");
}

/*!
 * \internal
 *
 * Reset the internal states. Required when the next response is parsed.
 */
void QsrMailTransportPrivate::SmtpResponse::reset()
{
    line.clear();
    lines.clear();
    code = 0;
    isValid = false;
    isEmpty = true;
    isCompleted = false;
    isIntermediate = false;
    isTransientError = false;
    isPermanentError = false;
    isSuccess = false;
    isError = false;
}

/*!
 * \internal
 *
 * Append the *data* response from the server to the structure. After the
 * function splitted the data it sets its members accordingly. The response
 * is considered complete when the isValid flag is set after this call. The
 * function returns false if *data* is not parsable.
 */
bool QsrMailTransportPrivate::SmtpResponse::append(const QByteArray &data)
{
    bool cont = false;
    const char *p = 0;
    const char *lp = 0;
    int s = 0;
    code = 0;

    for (p=data.constData(); *p && *p!='\r' && *p!='\n'; ) {
        char c = *p++;

        switch (s) {
        case 0: /* response code */
        case 1:
        case 2:
            if (c < '0' || c > '9')
                return false;
            code *= 10;
            code += c - '0';
            s++;
            break;

        case 3: /* continuation */
            if (c == '-')
                cont = true;
            else if (c != ' ')
                return false;
            lp = p;
            s++;
            break;
        }
    }

    if (s != 4)
        return false;

    isCompleted = code >= 200 && code < 300;
    isIntermediate = code >= 300 && code < 400;
    isTransientError = code >= 400 && code < 500;
    isPermanentError = code >= 500 && code < 600;
    isSuccess = code >= 200 && code < 400;
    isError = code >= 400 && code < 600;

    line = QByteArray(lp, static_cast<int>(p-lp));
    lines.append(line);
    if (!cont)
        isValid = true;

    isEmpty = false;
    return true;
}

/* -------------------------------------------------------------------------- */

/*!
 * Construct a new mail transport. Optionally assign a *parent* to
 * the object.
 */
QsrMailTransport::QsrMailTransport(QObject *parent) :
    QObject(parent),
    d_ptr(new QsrMailTransportPrivate(this))
{
    Q_D(QsrMailTransport);

    /* setup the socket */
    d->socket = new QSslSocket(this);
    connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(_q_stateChanged(QAbstractSocket::SocketState)));
    connect(d->socket, SIGNAL(readyRead()),
            this, SLOT(_q_socketReadyRead()));
    connect(d->socket, SIGNAL(encryptedBytesWritten(qint64)),
            this, SLOT(_q_encryptedBytesWritten(qint64)));
    connect(d->socket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(_q_bytesWritten(qint64)));



    /* setup the timer */
    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(_q_timeout()));

    d->timer->setSingleShot(true);
}

/*!
 * \internal
 *
 * Constructor for derived classes. Constructs an instance from *qq* and
 * *parent*.
 */
QsrMailTransport::QsrMailTransport(QsrMailTransportPrivate &qq,
                                   QObject *parent) :
    QObject(parent),
    d_ptr(&qq)
{
}

/*!
 * Destroys the instance
 */
QsrMailTransport::~QsrMailTransport()
{
}

/*!
 * Set the *username* used for authentication with the SMTP server.
 */
void QsrMailTransport::setUser(const QString &username)
{
    Q_D(QsrMailTransport);
    d->username = username;
}

/*!
 * Returns the currently set username for authentication.
 */
QString QsrMailTransport::user() const
{
    Q_D(const QsrMailTransport);
    return d->username;
}

/*!
 * Set the *password* used for authentication with the SMTP server.
 * Please be aware that the password is stored in **cleartext** within
 * the object in a not particullary safe storage.
 */
void QsrMailTransport::setPassword(const QString &passwd)
{
    Q_D(QsrMailTransport);
    d->password = passwd;
}

/*!
 * Returns the currently set password for authentication
 */
QString QsrMailTransport::password() const
{
    Q_D(const QsrMailTransport);
    return d->password;
}

/*!
 * Set the authentication mechanism. The default is to use autodetection
 * of the best possible mechanism. The detection checks for CRAM-MD5,
 * LOGIN and PLAIN in that order and selects the first mechanism the
 * server supports.
 *
 * If an authentication mechanism is explicitly chosen by the developer
 * and the server does not support the authentication then the
 * authentication will fail.
 */
void QsrMailTransport::setAuthMech(AuthMech mechanism)
{
    Q_D(QsrMailTransport);
    d->authMech = mechanism;
}

/*!
 * Returns the current authentication mechanism.
 */
QsrMailTransport::AuthMech QsrMailTransport::authMech() const
{
    Q_D(const QsrMailTransport);
    return d->authMech;
}

/*!
 * Set the system *identifier* for the SMTP dialog. The system identifier
 * is presented to the SMTP server during the EHLO/HELO dialog. If not set
 * the system identifier defaults to *localhost*.
 */
void QsrMailTransport::setSystemIdentifier(const QByteArray &value)
{
    Q_D(QsrMailTransport);
    d->systemIdentifier = value;
}

/*!
 * Return the currently set system identifier.
 */
QByteArray QsrMailTransport::systemIdentifier() const
{
    Q_D(const QsrMailTransport);
    return d->systemIdentifier;
}

/*!
 * This is the timeout for the communication with the SMTP server. If the
 * server does not response within the given timeout (which is specified in
 * milliseconds) the delivery fails and all messages will yield an error.
 * Changing the property during mail delivery is not supported and may
 * lead to unexpected results. If not specified the timeout is set to
 * 60000 (equals 60 seconds).
 */
void QsrMailTransport::setTimeout(int timeout)
{
    Q_D(QsrMailTransport);
    d->timeout = timeout;
}

/*!
 * Return the current timeout for server communication.
 */
int QsrMailTransport::timeout() const
{
    Q_D(const QsrMailTransport);
    return d->timeout;
}

/*!
 * Set the required encryption *level* for the connection. The level can be:
 *
 * * TlsDisabled - which disables all encryption
 * * TlsOptional - which makes encryption optional
 * * TlsRequired - which makes encryption a must
 *
 * Changing the property during mail delivery is not supported and may
 * lead to unexpected results. If not specified the tls level is set to
 * TlsOptional which is a reasonable setting.
 */
void QsrMailTransport::setTlsLevel(TlsLevel level)
{
    Q_D(QsrMailTransport);
    d->tlsLevel = level;
}

/*!
 * Return the currently set encryption level.
 */
QsrMailTransport::TlsLevel QsrMailTransport::tlsLevel() const
{
    Q_D(const QsrMailTransport);
    return d->tlsLevel;
}

/*!
 * Use this to set the ssl configuration used in the connection. Using
 * this property the developer decides the encryption parameters and the
 * level of trust (certificate authentication) for the connection.
 * Changing the property during mail delivery is not supported and may
 * lead to unexpected results. If not specified the Qt defaults will be
 * used.
 *
 * \sa QSslConfiguration
 */
void QsrMailTransport::setSslConfiguration(const QSslConfiguration &value)
{
    Q_D(QsrMailTransport);
    d->socket->setSslConfiguration(value);
}

/*!
 * Return the currently used ssl configuration.
 */
QSslConfiguration QsrMailTransport::sslConfiguration() const
{
    Q_D(const QsrMailTransport);
    return d->socket->sslConfiguration();
}

/*!
 * Add *message* to the queue of messages which should be delivered to the
 * SMTP server. To deliver the mail queue use sendMessages().
 * Adding messages during mail delivery is not supported and will lead
 * to unexpected results (namely memory corruption).
 *
 * The function returns a QsrMailTransaction object which is used to track
 * the delivery. It is up to the developer to dispose the object once it
 * is no longer required (usually when the QsrMailTransaction::finished()
 * signal fires).
 */
QsrMailTransaction * QsrMailTransport::queueMessage(
        const QsrMailMessage &message)
{
    Q_D(QsrMailTransport);
    return d->queueMessageImpl(message);
}

/*!
 * Start the mail delivery of the messages.
 *
 * The delivery starts by connecting the server at *serverHostname*
 * using port *serverPort*. After successful protocol handshake and
 * authentication the queued messages are delivered to the server. The
 * finished() signal is emitted when all messages have been processed.
 *
 * Port 25 is set as default since this is the most common port. However
 * port 587 is also very popular for mail submission using authentication.
 *
 * Make sure you do **not** call this function when a mail delivery is
 * already running. Keep track of the finished() signal!
 */
void QsrMailTransport::sendMessages(const QString &serverHostname, quint16 port,
                                    QAbstractSocket::NetworkLayerProtocol protocol)
{
    Q_D(QsrMailTransport);

    /* setup input states and start the fsm */
    d->serverHostname = serverHostname;
    d->serverProtocol = protocol;
    d->serverAddress = QHostAddress();
    d->serverPort = port;

    d->sendMessagesImpl(QsrMailTransportPrivate::ResolvingState);
}

/*!
 * This is an overloaded version providing an interface using QHostAddress
 * as *serverAddress* instead of a hostname.
 */
void QsrMailTransport::sendMessages(const QHostAddress &serverAddress,
                                    quint16 port)
{
    Q_D(QsrMailTransport);

    d->serverHostname = QString();
    d->serverProtocol = serverAddress.protocol();
    d->serverAddress = serverAddress;
    d->serverPort = port;

    d->sendMessagesImpl(QsrMailTransportPrivate::ConnectedState);
}

/*!
 * Abort the current transfer. Immediatly aborts the connection and flushes
 * the queue with the QsrMailTransaction::AbortedError code.
 */
void QsrMailTransport::abort()
{
    Q_D(QsrMailTransport);

    d->aborted = true;
    d->socket->disconnectFromHost();
}

QT_END_NAMESPACE

#include "moc_qsrmailtransport.cpp"
