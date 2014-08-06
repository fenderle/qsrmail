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
 * \class QsrMailMimeMultipart qsrmailmimemultipart.h <QsrMailMimeMultipart>
 * \brief This class wraps one or more multipart part and defines the
 * relationship between the parts.
 *
 * The structure can be nested so that a multipart can accompany other
 * multiparts or the parts itself. The structure depends on the requirements for
 * the message. A simple attachment requires only one level of multipart
 * structure. For a more detailed explanation on the relationship of multiparts
 * and parts please refer to RFC2046 section 5.
 *
 * As with QsrMailMessage it is possible to inject raw headers into the
 * multipart compound. The same handling rules apply here.
 *
 * \sa QsrMailMessage, QsrMailMimePart
 */

/*!
 * \enum QsrMailMimeMultipart::ContentType
 * Multipart MIME parts utilize special and well defined content types. This
 * enum defines them.
 *
 * \var QsrMailMimeMultipart::MixedType
 * The parts in this multipart have no direct relationship. They are
 * independent to each other. However the order of the parts is significant.
 * This is the type which is used for attachments.
 *
 * \var QsrMailMimeMultipart::AlternativeType
 * The parts are interchangeable, but usually offer different representations
 * of the same content. For example plain text and html versions of the mail
 * body. Generally the client picks the version which best fits its
 * possibilities. The order is significant with increasing confidence towards
 * the end of the part list. Thus a client normally chooses the last part of
 * the list it can display correctly.
 *
 * \var QsrMailMimeMultipart::DigestType
 * The parts represent different RFC822 messages and can be used to bundle
 * more messages together.
 *
 * \var QsrMailMimeMultipart::ParallelType
 * This one is similar to QsrMailMimeMultipart::AlternativeType with the
 * exception that all parts are displayed at the same time if the client is
 * capable of doing so.
 */

#include "qsrmailmimemultipart.h"
#include "qsrmailabstractpart_p.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QSharedDataPointer<QsrMailAbstractPartPrivate>,
                          sharedNull, (new QsrMailAbstractPartPrivate(
                                       QsrMailAbstractPart::MimeMultipartType)));

/*!
 * Constructs an empty multipart compound with content type mixed.
 */
QsrMailMimeMultipart::QsrMailMimeMultipart() :
    QsrMailAbstractMimePart(*sharedNull)
{
}

/*!
 * Constructs an empty multipart compound with the supplied content type of
 * *type*.
 */
QsrMailMimeMultipart::QsrMailMimeMultipart(ContentType type) :
    QsrMailAbstractMimePart(new QsrMailAbstractPartPrivate(
                                QsrMailAbstractPart::MimeMultipartType))
{
    d->multipartContentType = type;
}

/*!
 * \copydoc QsrMailAbstractPart::QsrMailAbstractPart(const QsrMailAbstractPart &)
 */
QsrMailMimeMultipart::QsrMailMimeMultipart(const QsrMailMimeMultipart &other) :
    QsrMailAbstractMimePart(other)
{
}

/*!
 * Destroy the instance.
 */
QsrMailMimeMultipart::~QsrMailMimeMultipart()
{
}

/*!
 * \copydoc QsrMailAbstractPart::operator=(const QsrMailAbstractPart &)
 */
QsrMailMimeMultipart &
QsrMailMimeMultipart::operator=(const QsrMailMimeMultipart &other)
{
    if (d != other.d) {
        QsrMailMimeMultipart tmp(other);
        tmp.swap(*this);
    }

    return *this;
}

/*!
 * Swap this instance with *other*. This function is very fast and
 * never fails.
 */
void QsrMailMimeMultipart::swap(QsrMailMimeMultipart &other)
{
    qSwap(d, other.d);
}

/*!
 * \copydoc QsrMailAbstractPart::isNull()
 */
bool QsrMailMimeMultipart::isNull() const
{
    return d == *sharedNull;
}

/*!
 * Set the content type of the multipart to *contentType*. Selecting
 * the correct content type is essential for representing the message
 * in the mail client as intented.
 *
 * \sa QsrMailMimeMultipart::ContentType
 * [RFC2046 section 5.1](http://tools.ietf.org/html/rfc2046#section-5.1)
 */
void QsrMailMimeMultipart::setContentType(ContentType contentType)
{
    d->multipartContentType = contentType;
}

/*!
 * Returns the content type for this multipart.
 */
QsrMailMimeMultipart::ContentType QsrMailMimeMultipart::contentType() const
{
    return d->multipartContentType;
}

/*!
 * Define the *boundary* of this multipart compound. The boundary is used
 * to separate the several parts of the multipart. You do not have to set
 * this if you are fine with the automatically generated boundary which is
 * actually based on an UUID and generated on instanciation.
 *
 * \sa [RFC2046 section 5.1.1](http://tools.ietf.org/html/rfc2046#section-5.1.1)
 */
void QsrMailMimeMultipart::setBoundary(const QByteArray &boundary)
{
    d->boundary = boundary;
}

/*!
 * Returns the current boundary for this multipart compound.
 */
QByteArray QsrMailMimeMultipart::boundary() const
{
    return d->boundary;
}

/*!
 * Append a mime *part* to this multipart compound. This can be either
 * a QsrMailMimePart or a QsrMailMimeMultipart.
 */
void QsrMailMimeMultipart::append(const QsrMailAbstractMimePart &part)
{
    d->parts.append(part);
}

/*!
 * Return a list of the parts attached to this compound. The items of the
 * list will be either of MimePartType or MimeMultipartType.
 */
QList<QsrMailAbstractPart> QsrMailMimeMultipart::parts() const
{
    return d->parts;
}

QT_END_NAMESPACE
