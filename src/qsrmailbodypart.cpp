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
 * \class QsrMailBodyPart qsrmailbodypart.h <QsrMailBodyPart>
 * \brief QsrMailBodyPart is used when a mailbody needs to be send directly
 * without further modifications.
 *
 * This is of course a special use case since most of the time the messages will
 * be using MIME. However, for completeness, this is the way to go if raw data
 * should.
 *
 * \warning
 * Make sure that the raw data does not contain the CRLF.CRLF sequence, since
 * this will almost certainly break the message transfer to the server.
 *
 * For more information of mail body encoding take a look at
 * [RFC5322 section 2.3](http://tools.ietf.org/html/rfc5322#section-2.3)
 */

#include "qsrmailbodypart.h"
#include "qsrmailabstractpart_p.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QSharedDataPointer<QsrMailAbstractPartPrivate>,
                          sharedNull, (new QsrMailAbstractPartPrivate(
                                       QsrMailAbstractPart::BodyPartType)));

/*!
 * Default constructor.
 */
QsrMailBodyPart::QsrMailBodyPart() :
    QsrMailAbstractPart(*sharedNull)
{
}

/*!
 * \copydoc QsrMailAbstractPart::QsrMailAbstractPart(const QsrMailAbstractPart &)
 */
QsrMailBodyPart::QsrMailBodyPart(const QsrMailBodyPart &other) :
    QsrMailAbstractPart(other)
{
}

/*!
 * \copydoc QsrMailAbstractPart::operator=(const QsrMailAbstractPart &)
 */
QsrMailBodyPart &QsrMailBodyPart::operator=(const QsrMailBodyPart &other)
{
    if (d != other.d) {
        QsrMailBodyPart tmp(other);
        tmp.swap(*this);
    }

    return *this;
}

/*!
 * Swap this instance with *other*. This function is very fast and
 * never fails.
 */
void QsrMailBodyPart::swap(QsrMailBodyPart &other)
{
    qSwap(d, other.d);
}

/*!
 * Destroy the instance.
 */
QsrMailBodyPart::~QsrMailBodyPart()
{
}

/*!
 * \copydoc QsrMailAbstractPart::isNull()
 */
bool QsrMailBodyPart::isNull() const
{
    return d == *sharedNull;
}

/*!
 * Controls the autoDelete mechanism. If autoDelete is enabled the bodyDevice
 * will be closed and deleted after use using a deleteLater() call on the
 * QIODevice. The autoDelete flag defaults to false so the object does usually
 * not take ownership.
 */
void QsrMailBodyPart::setAutoDelete(bool enabled)
{
    d->autoDelete = enabled;
}

/*!
 * Returns true if autoDelete is enabled, false otherwise.
 */
bool QsrMailBodyPart::autoDelete() const
{
    return d->autoDelete;
}

/*!
 * Set *content* as body for this part.
 *
 * \warning
 * Make sure that the raw data does not contain the CRLF.CRLF sequence, since
 * this will almost certainly break the message transfer to the server.
 */
void QsrMailBodyPart::setBody(const QByteArray &content)
{
    d->body = content;
}

/*!
 * Returns the body for this part. If the body is not set or the body is
 * a device a default constructed QByteArray is returned.
 */
QByteArray QsrMailBodyPart::body() const
{
    return d->body;
}

/*!
 * Set *device* as body for this part. The device must be opened and readable.
 * Using the body device overrides the use of setBody(). For unsetting the
 * device use setBodyDevice(0). The ownership control can be set using the
 * autoDelete flag.
 *
 * \warning
 * Make sure that the raw data does not contain the CRLF.CRLF sequence, since
 * this will almost certainly break the message transfer to the server.
 *
 * \sa
 * setAutoDelete(), autoDelete()
 */
void QsrMailBodyPart::setBodyDevice(QIODevice *device)
{
    d->bodyDevice = device;
}

/*!
 * Returns the device for this part. If the device is not set or the body
 * has been set by setBody(), null will be returned.
 */
QIODevice *QsrMailBodyPart::bodyDevice() const
{
    return d->bodyDevice;
}

/*!
 * Construct a part from a *data* block.
 *
 * \warning
 * Make sure that the raw data does not contain the CRLF.CRLF sequence, since
 * this will almost certainly break the message transfer to the server.
 */
QsrMailBodyPart QsrMailBodyPart::fromRawData(const QByteArray &data)
{
    QsrMailBodyPart result;

    result.d->body = data;

    return result;
}

QT_END_NAMESPACE
