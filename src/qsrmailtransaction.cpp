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
 * \class QsrMailTransaction qsrmailtransaction.h <QsrMailTransaction>
 * \brief A transaction object which is handed to the developer to track the
 * state of a mail transfer.
 *
 * The system is similar to QNetworkReply. In it's lifecycle the object gets
 * instantiated by QsrMailTransport::queueMessage(). When the message is
 * actually delivered using QsrMailTransport::sendMessages() it starts emitting
 * progressUpdate() signals. Once the message is completed the finished() signal
 * is emitted. Since the finished() signal is also emitted for errornous message
 * deliveries the developer has to check the error() code for the success of the
 * transfer. Additional information can be obtained from errorText(), status()
 * and statusText().
 *
 * The transfer of a message can be aborted any time by invoking the abort()
 * slot.
 */

/*!
 * \enum QsrMailTransaction::TransactionError
 * This enum is used to describe error codes which can be a result of
 * an finished QsrMailTransaction.
 *
 * \var QsrMailTransaction::NoError
 * Indicates the successful transfer of the message to the server.
 *
 * \var QsrMailTransaction::NoSenderError
 * The message has no sender information. The sender is mandatory.
 *
 * \var QsrMailTransaction::NoRecipientsError
 * The message has no recipient information. At least one recipient is
 * mandatory.
 *
 * \var QsrMailTransaction::ResponseError
 * The server replied with an error to one of the commands. statusText()
 * contains additional information.
 *
 * \var QsrMailTransaction::ConnectionError
 * The communication channel failed on a lower level (eg. the server closed
 * the connection unexpectedly). errorText() contains additional information.
 *
 * \var QsrMailTransaction::TlsRequiredError
 * TLS is set as mandatory for this connection, but the server does not offer
 * TLS encryption.
 *
 * \var QsrMailTransaction::ResolverError
 * The hostname could not be resolved. errorText() contains the error message
 * from the resolver.
 *
 * \var QsrMailTransaction::TimeoutError
 * The server did not respond within the given timeframe and the connection
 * has been drop.
 *
 * \var QsrMailTransaction::AbortedError
 * The message has been aborted by the user using the abort() slot.
 *
 * \var QsrMailTransaction::DataError
 * For some reason the data renderer could not render the message. errorText()
 * contains details.
 */

/*!
 * \fn QsrMailTransaction::finished()
 *
 * This signal is emitted when a message transfer completes. It does not
 * matter if the transfer was successfull or not - the signal is emitted in
 * either case.
 */

/*!
 * \fn QsrMailTransaction::error(QsrMailTransaction::TransactionError error)
 *
 * This signal is emitted right before the finished signal if the transaction
 * encountered an error.
 */

/*!
 * \fn QsrMailTransaction::progressUpdate(int percent)
 *
 * Reflects the sending progress of the message normalized to 100%. This signal
 * is intented for UI purposes to display some sort of progress to the user.
 */

/*!
 * \internal
 *
 * \class QsrMailTransactionPrivate qsrmailtransaction_p.h
 * \brief The private data class of the QsrMailTransaction class.
 */

#include "qsrmailtransaction.h"
#include "qsrmailtransaction_p.h"

#include "qsrmailtransport.h"

#include <QStringBuilder>

QT_BEGIN_NAMESPACE

/*!
 * \internal
 *
 * Construct a data class for QsrMailTransactionPrivate from *qq*.
 */
QsrMailTransactionPrivate::QsrMailTransactionPrivate(QsrMailTransaction *qq) :
    q_ptr(qq),
    error(QsrMailTransaction::NoError),
    status(0),
    encrypted(false),
    authenticated(false)
{
}

/*!
 * \internal
 *
 * Finalize the transaction. This is called from the transport and emits the
 * finished() and optionally the error() signal, depending on the error code
 * of the transaction.
 */
void QsrMailTransactionPrivate::finalize()
{
    Q_Q(QsrMailTransaction);

    if (error != QsrMailTransaction::NoError)
        emit q->error(error);

    emit q->finished();
}

/*!
 * \internal
 *
 * Set the transaction error *code* and it's *text* representation. If *text*
 * is a default constructed QString() a standard text will be used instead.
 */
void QsrMailTransactionPrivate::setError(QsrMailTransaction::TransactionError code,
                                         const QString &text)
{
    error = code;
    errorText = text;

    if (errorText.isNull()) {
        switch (code) {
        case QsrMailTransaction::NoError:
            errorText = QLatin1String("No error occured");
            break;

        case QsrMailTransaction::NoSenderError:
            errorText = QLatin1String("No sender/from has been specified");
            break;

        case QsrMailTransaction::NoRecipientsError:
            errorText = QLatin1String("No recipients have been specified");
            break;

        case QsrMailTransaction::ResponseError:
            errorText = QLatin1String("Unexpected server response");
            break;

        case QsrMailTransaction::ConnectionError:
            errorText = QLatin1String("The connection timed out or the remote " \
                                      "server unexpectedly closed the connection");
            break;

        case QsrMailTransaction::TlsRequiredError:
            errorText = QLatin1String("TLS required but not available");
            break;

        case QsrMailTransaction::ResolverError:
            errorText = QLatin1String("Unable to resolve hostname");
            break;

        case QsrMailTransaction::TimeoutError:
            errorText = QLatin1String("Connection dropped by timeout");
            break;

        case QsrMailTransaction::AbortedError:
            errorText = QLatin1String("Message aborted.");
            break;

        case QsrMailTransaction::DataError:
            errorText = QLatin1String("Message cannot be rendered.");
            break;
        }
    }
}

/*!
 * \internal
 *
 * Set the server status response to *code* and it's *text* representation.
 * The multiple lines are joined by a SPC character to form a single line.
 */
void QsrMailTransactionPrivate::setStatus(int code, const QList<QByteArray> &text)
{
    status = code;

    if (text.size() < 2) {
        /* shortcut for single line */
        statusText = QString::fromLatin1(text.value(0));
    } else {
        QByteArray buffer;
        foreach (const QByteArray &line, text)
            buffer = buffer % line % ' ';
        buffer.chop(1);
        statusText = QString::fromLatin1(buffer);
    }
}

/*!
 * \internal
 *
 * Set the transaction delivery progress to percent. This emits the
 * progressUpdate() signal.
 */
void QsrMailTransactionPrivate::setProgress(int percent)
{
    Q_Q(QsrMailTransaction);
    emit q->progressUpdate(percent);
}

/*!
 * \internal
 *
 * Reutrns a new QsrMailTransaction instance. The new instance will
 * be a child of *transport*. The message *msg* will be set as the content
 * of the transaction. A QsrMailRenderer is instantiated with this
 * transaction as its parent QObject and *msg* as data source.
 */
QsrMailTransaction *
QsrMailTransactionPrivate::createInstance(const QsrMailMessage &message,
                                          QsrMailTransport *transport)
{
    QsrMailTransaction *t = new QsrMailTransaction(transport);
    QsrMailTransactionPrivate *p = t->d_func();

    /* setup data for the message */
    p->transport = transport;
    p->renderer = new QsrMailRenderer(message, t);
    p->message = message;

    return t;
}

/* -------------------------------------------------------------------------- */

/*!
 * Creates a new QsrMailTransaction as child of *parent*. You cannot directly
 * instantiate QsrMailTransaction objects. Use QsrMailTransport to do that.
 */
QsrMailTransaction::QsrMailTransaction(QsrMailTransport *transport) :
    QObject(transport),
    d_ptr(new QsrMailTransactionPrivate(this))
{
    qRegisterMetaType<QsrMailTransaction::TransactionError>();
}

/*!
 * Destroys the instance.
 */
QsrMailTransaction::~QsrMailTransaction()
{
}

/*!
 * Return the QsrMailTransport associated with this object
 */
QsrMailTransport *QsrMailTransaction::transport() const
{
    Q_D(const QsrMailTransaction);
    return d->transport;
}

/*!
 * Returns the QsrMailMessage associated with the transaction.
 */
QsrMailMessage QsrMailTransaction::message() const
{
    Q_D(const QsrMailTransaction);
    return d->message;
}

/*!
 * Returns the error value. The error value is only valid after the
 * finished() signal has been emitted.
 */
QsrMailTransaction::TransactionError QsrMailTransaction::error() const
{
    Q_D(const QsrMailTransaction);
    return d->error;
}

/*!
 * Returns the textual representation of the error() value. The text might
 * also contain additional information for the error (so it is not always
 * a static text per error code).
 */
QString QsrMailTransaction::errorText() const
{
    Q_D(const QsrMailTransaction);
    return d->errorText;
}

/*!
 * Return the status code of the server response. This is the status code of
 * the last server response after the finished() signal is emitted.
 */
int QsrMailTransaction::status() const
{
    Q_D(const QsrMailTransaction);
    return d->status;
}

/*!
 * Returns the textual part of the server response. This contains often useful
 * information, even for 200 status codes where the almost every server sends
 * the queue id of the message on the server side. For debugging it is a good
 * technique to log status() and statusText(), even if error() returns NoError
 * and indicates success.
 */
QString QsrMailTransaction::statusText() const
{
    Q_D(const QsrMailTransaction);
    return d->statusText;
}

/*!
 * Returns true if the transport connection was encryption.
 */
bool QsrMailTransaction::isEncrypted() const
{
    Q_D(const QsrMailTransaction);
    return d->encrypted;
}

/*!
 * Returns the sockets ssl configuration at the time the transaction was
 * transmitted.
 */
QSslConfiguration QsrMailTransaction::sslConfiguration() const
{
    Q_D(const QsrMailTransaction);
    return d->sslConfiguration;
}

/*!
 * Returns true if the transport connection was authenticated.
 */
bool QsrMailTransaction::isAuthenticated() const
{
    Q_D(const QsrMailTransaction);
    return d->authenticated;
}

/*!
 * Returns the authentication mech used for authentication.
 */
QsrMailTransport::AuthMech QsrMailTransaction::authMech() const
{
    Q_D(const QsrMailTransaction);
    return d->authMech;
}

/*!
 * Returns the username used for authentication.
 */
QString QsrMailTransaction::username() const
{
    Q_D(const QsrMailTransaction);
    return d->username;
}

/*!
 * Abort the message. Immediatly aborts the message and emits the finished
 * signal.
 */
void QsrMailTransaction::abort()
{
    Q_D(QsrMailTransaction);
    d->setError(AbortedError);
    d->finalize();
}

QT_END_NAMESPACE

#include "moc_qsrmailtransaction.cpp"
