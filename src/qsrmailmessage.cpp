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
 * \class QsrMailMessage qsrmailmessage.h <QsrMailMessage>
 * \brief This shared data class describes a mail message.
 *
 * It's the topmost class required to prepare a message for submission to the
 * transport. Besides the obvious properties like sender, recipients, subject,
 * date, etc. it also holds either a mime message body which is constructed from
 * QsrMailMimeMultipart like classes or a raw (QByteArray) body in case you
 * want to assemble the message data yourself. The suggested way is to use the
 * QsrMailMimeMultipart classes.
 *
 * The class also offers so called 'raw headers' which exist to offer a way
 * for injecting own headers. These headers live side by side with the standard
 * headers (like Subject or From) for which the class offers a direct interface.
 * You should always use the standard methods for setting headers if they are
 * available since these interfaces provide automatic conversion for the
 * headers. If you really need to set a header which is not part of the
 * property set you can fallback to the raw headers. But keep in mind that it
 * is your responsibility to format the header values according to the RFC
 * requirements. If there are collision with properties and raw headers, the
 * properties (if set) take precedence over raw headers, meaning that single
 * headers (eg. Subject) *replace* the raw headers with the same name while
 * list headers (eg. To) get *appended* to the raw headers.
 *
 * Example:
 * \code
 * QsrMailMessage msg;
 * msg.setFrom(QsrMailAddress("someone@local.foo"));
 * msg.setTo(QsrMailAddress("anotherone@local.foo"));
 * msg.setSubject("Just a test");
 *
 * QsrMailMimePart body;
 * body.setBody("Hello world");
 * msg.setMimeBody(body);
 *
 * // msg now holds a valid, sendable message
 * \endcode
 *
 * \sa QsrMailMimeMultipart, QsrMailTransport
 */

/*!
 * \internal
 *
 * \class QsrMailMessagePrivate "qsrmailmessage_p.h"
 * \brief Private data class for QsrMailMessage.
 */

#include "qsrmailmessage.h"
#include "qsrmailmessage_p.h"
#include "qsrmailrfctools_p.h"

#include <QUuid>
#include <QHostInfo>
#include <QStringBuilder>

QT_BEGIN_NAMESPACE

/*!
 * \internal
 *
 * Default constructor.
 */
QsrMailMessagePrivate::QsrMailMessagePrivate()
{
}

/*!
 * \internal
 *
 * Returns a list of cooked headers which incorporate all the internet
 * message headers unified from the data of the object.
 *
 * The result is initialized with the raw headers and then unified with
 * the properties where all list properties get appended to the result
 * and single instances get replaced (eg. Subject, Date) by the property
 * value.
 *
 * An exemption is the Date header which falls back to the current date
 * time if neither a raw header nor the date property has been set.
 *
 * All returned headers are appropriately formatted.
 */
QsrMailHeaders QsrMailMessagePrivate::cookHeaders() const
{
    /* preset with the raw headers */
    QsrMailHeaders result(headers);

    /* add From addresses */
    for (int i=0, size=from.size(); i<size; ++i)
        result.appendHeader("From", from.at(i).toByteArray());

    /* add To addresses */
    for (int i=0, size=to.size(); i<size; ++i)
        result.appendHeader("To", to.at(i).toByteArray());

    /* add Reply-To addresses */
    for (int i=0, size=replyTo.size(); i<size; ++i)
        result.appendHeader("Reply-To", replyTo.at(i).toByteArray());

    /* add Cc addresses */
    for (int i=0, size=cc.size(); i<size; ++i)
        result.appendHeader("Cc", cc.at(i).toByteArray());

    /* add Bcc addresses */
    for (int i=0, size=bcc.size(); i<size; ++i)
        result.appendHeader("Bcc", bcc.at(i).toByteArray());

    /* add date/time, fallback to current date time */
    if (date.isValid())
        result.setHeader("Date", QsrMailRfcTools::rfc2822Date(date));
    else if (!result.hasHeader("Date"))
        result.setHeader("Date", QsrMailRfcTools::rfc2822Date(QDateTime::currentDateTime()));

    /* set Subject */
    if (!subject.isNull())
        result.setHeader("Subject", QsrMailRfcTools::toEncodedWords(subject));

    /* add message id */
    result.setHeader("Message-ID", messageId);

    /* add user agent if not supplied */
    if (!result.hasHeader("User-Agent"))
        result.setHeader("User-Agent", "QsrMail " QSRMAIL_VERSION_STR);

    return result;
}

/* -------------------------------------------------------------------------- */

/*!
 * Constructs an empty mail message.
 */
QsrMailMessage::QsrMailMessage() :
    d(new QsrMailMessagePrivate)
{
    /* create a default messageId */
    QByteArray uuid(QUuid::createUuid().toRfc4122());
    QString host(QHostInfo::localHostName().split('.').value(0, "unknown"));
    d->messageId = '<' % uuid.toHex() % '@' % host.toLatin1() % '>';
}

/*!
 * Construct a copy of *other*. This operation is fast and takes constant
 * time since QsrMailMessage is implicitly shared. Write operations on
 * the shared data detaches it (copy-on-write).
 */
QsrMailMessage::QsrMailMessage(const QsrMailMessage &other) :
    d(other.d)
{
}

/*!
 * Assigns *other* to this QsrMailMessage and returns a pointer to this
 * instance.
 */
QsrMailMessage &QsrMailMessage::operator=(const QsrMailMessage &other)
{
    if (d != other.d) {
        QsrMailMessage tmp(other);
        tmp.swap(*this);
    }

    return *this;
}

/*!
 * Swap this instance with *other*. This function is very fast and
 * never fails.
 */
void QsrMailMessage::swap(QsrMailMessage &other)
{
    qSwap(d, other.d);
}

/*!
 * Destroys the instance.
 */
QsrMailMessage::~QsrMailMessage()
{
}

/*!
 * Returns *true* if the message does not contain any data. This
 * includes headers and body.
 */
bool QsrMailMessage::isEmpty() const
{
    return d->messageId.isEmpty()
            && d->headers.headers.isEmpty()
            && d->body.isEmpty();
}

/*!
 * Set a specific message id used for identifying the message in logs,
 * etc. Use this property if the system should not autogenerate a
 * message id while delivering the but use the supplied one. The message
 * id however must confirm to RFC standard.
 */
void QsrMailMessage::setMessageId(const QByteArray &messageId)
{
    d->messageId = messageId;
}

/*!
 * Returns the message id for the message.
 */
QByteArray QsrMailMessage::messageId() const
{
    return d->messageId;
}

/*!
 * Sets an unprocessed header named *name* with the value *value* in the
 * list of headers. If the header already exists it will be replaced. The
 * header is written directly to the wire as part of the message headers
 * without further processing. The header itself must confirm to RFC
 * standards.
 */
void QsrMailMessage::setRawHeader(const QByteArray &name,
                                  const QByteArray &value)
{
    d->headers.setHeader(name,value);
}

/*!
 * Append an unprocessed header named *name* with the value *value* to the
 * list of headers for this message.
 */
void QsrMailMessage::appendRawHeader(const QByteArray &name,
                                     const QByteArray &value)
{
    d->headers.appendHeader(name, value);
}

/*!
 * Return the value of the first header named *name*. If no header is found
 * a null QByteArray is returned.
 */
QByteArray QsrMailMessage::rawHeader(const QByteArray &name) const
{
    return d->headers.value(name);
}

/*!
 * Return a list of values for all headers named *name*.
 */
QList<QByteArray> QsrMailMessage::rawHeaders(const QByteArray &name) const
{
    return d->headers.values(name);
}

/*!
 * Use *senderAddress* as the address supplied for the FROM command in the
 * SMTP dialogue with the server. The server uses this address for routing
 * the message.
 *
 * Usually it is *not* required to set since most of the time the from
 * address is also the sender address. In case of multiple from addresses
 * the first from address is used as the sender address.
 */
void QsrMailMessage::setSender(const QsrMailAddress &senderAddress)
{
    d->sender = senderAddress;
}

/*!
 * Returns the *sender* address of this message
 */
QsrMailAddress QsrMailMessage::sender() const
{
    return d->sender;
}

/*!
 * Set the address for the From headers. This replaces the current list of
 * from addresses by *fromAddress*. Note that there may be more than one
 * from addresses (not widely known, but allowed by RFC).
 *
 * If no sender address is set the first from address is used as the sender
 * address.
 */
void QsrMailMessage::setFrom(const QsrMailAddress &fromAddress)
{
    d->from.clear();
    d->from.append(fromAddress);
}

/*!
 * Replace the list of from addresses for this message by *fromAddresses*.
 */
void QsrMailMessage::setFrom(const QList<QsrMailAddress> &fromAddresses)
{
    d->from = fromAddresses;
}

/*!
 * Append *fromAddress* to the list of from addresses.
 */
void QsrMailMessage::appendFrom(const QsrMailAddress &fromAddress)
{
    d->from.append(fromAddress);
}

/*!
 * Returns all *from* addresses of this message.
 */
QList<QsrMailAddress> QsrMailMessage::from() const
{
    return d->from;
}

/*!
 * Set the address for the To headers. This replaces the current list of
 * recipient addresses by *toAddress*.
 *
 * The recipient addresses are used in the mail message headers as well
 * as in the SMTP dialogue with the server for message routing.
 */
void QsrMailMessage::setTo(const QsrMailAddress &toAddress)
{
    d->to.clear();
    d->to.append(toAddress);
}

/*!
 * Replace the list of recipient addresses for this message by
 * *toAddresses*.
 */
void QsrMailMessage::setTo(const QList<QsrMailAddress> &toAddresses)
{
    d->to = toAddresses;
}

/*!
 * Returns all *to* addresses of this message.
 */
QList<QsrMailAddress> QsrMailMessage::to() const
{
    return d->to;
}

/*!
 * Append *toAddress* to the list of recipient addresses.
 */
void QsrMailMessage::appendTo(const QsrMailAddress &toAddress)
{
    d->to.append(toAddress);
}

/*!
 * Set the address for the Reply-To headers. This replaces the current
 * list of reply addresses by *replyToAddress*.
 */
void QsrMailMessage::setReplyTo(const QsrMailAddress &replyToAddress)
{
    d->replyTo.clear();
    d->replyTo.append(replyToAddress);
}

/*!
 * Replace the list of reply addresses for this message by
 * *replyToAddresses*.
 */
void QsrMailMessage::setReplyTo(const QList<QsrMailAddress> &replyToAddresses)
{
    d->replyTo = replyToAddresses;
}

/*!
 * Append *replyToAddress* to the list of reply addresses.
 */
void QsrMailMessage::appendReplyTo(const QsrMailAddress &replyToAddress)
{
    d->replyTo.append(replyToAddress);
}

/*!
 * Returns all *reply to* addresses of this message.
 */
QList<QsrMailAddress> QsrMailMessage::replyTo() const
{
    return d->replyTo;
}

/*!
 * Set the address for the Cc headers. This replaces the current list
 * of cc addresses by *ccAddress*. CC is an abbreviation for Carbon Copy.
 *
 * The cc addresses are used in the mail message headers as well
 * as in the SMTP dialogue with the server for message routing.
 */
void QsrMailMessage::setCc(const QsrMailAddress &ccAddress)
{
    d->cc.clear();
    d->cc.append(ccAddress);
}

/*!
 * Replace the list of cc addresses for this message by *ccAddresses*.
 */
void QsrMailMessage::setCc(const QList<QsrMailAddress> &ccAddresses)
{
    d->cc = ccAddresses;
}

/*!
 * Append *ccAddress* to the list of cc addresses.
 */
void QsrMailMessage::appendCc(const QsrMailAddress &ccAddress)
{
    d->cc.append(ccAddress);
}

/*!
 * Returns all *cc* addresses of this message.
 */
QList<QsrMailAddress> QsrMailMessage::cc() const
{
    return d->cc;
}

/*!
 * Set the address for the bcc recipients. This replaces the current list
 * of bcc addresses by *bccAddress*. BCC is an abbreviation for Blind
 * Carbon Copy.
 *
 * The bcc addresses are only used in the SMTP dialogue with the server
 * for message routing. They do not appear anywhere in the message itself.
 */
void QsrMailMessage::setBcc(const QsrMailAddress &bccAddress)
{
    d->bcc.clear();
    d->bcc.append(bccAddress);
}

/*!
 * Replace the list of bcc addresses for this message by *bccAddresses*.
 */
void QsrMailMessage::setBcc(const QList<QsrMailAddress> &bccAddresses)
{
    d->bcc = bccAddresses;
}

/*!
 * Append *bccAddress* to the list of bcc addresses.
 */
void QsrMailMessage::appendBcc(const QsrMailAddress &bccAddress)
{
    d->bcc.append(bccAddress);
}

/*!
 * Returns all *bcc* addresses of this message.
 */
QList<QsrMailAddress> QsrMailMessage::bcc() const
{
    return d->bcc;
}

/*!
 * Set the Date header to *date*. Since RFC2822 requires every message to
 * have a valid orig-date (aka. Date header) the current date and time is
 * used if an invalid (or null) QDateTime is supplied using setDate().
 */
void QsrMailMessage::setDate(const QDateTime &date)
{
    d->date = date;
}

/*!
 * Return the date set for the message
 */
QDateTime QsrMailMessage::date() const
{
    return d->date;
}

/*!
 * Set the message subject to *subject*. The subject may contain UTF-8
 * characters which are encoded on the fly by the transport engine.
 */
void QsrMailMessage::setSubject(const QString &subject)
{
    d->subject = subject;
}

/*!
 * Return the subject set for the message
 */
QString QsrMailMessage::subject() const
{
    return d->subject;
}

/*!
 * Set the body of the message. The body can consist of a QsrMailBodyPart
 * which is a raw body (not recomended, advance use only) or a
 * QsrMailMimeMultipart object which is the usual way to build messages. This
 * is the way to go for having attachments or other more complex messages.
 * For your convenience you can also supply a QsrMailMimePart directly, without
 * having to wrap it into the QsrMailMimeMultipart object.
 */
void QsrMailMessage::setBody(const QsrMailAbstractPart &part)
{
    d->body = part;
}

/*!
 * Returns the body part of this message.
 */
QsrMailAbstractPart QsrMailMessage::body() const
{
    return d->body;
}

QT_END_NAMESPACE
