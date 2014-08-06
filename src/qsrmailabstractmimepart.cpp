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
 * \class QsrMailAbstractMimePart "qsrmailabstractmimepart.h" <QsrMailAbstractMimePart>
 * \brief Interface class for all QsrMailMime*Part implementations.
 */

/*!
 * \enum QsrMailAbstractMimePart::DispositionType
 * This enum describes the typ of the content disposition. Each part can
 * have a Content-Disposition header which tells the MUA how to present
 * the part to the user.
 *
 * \var QsrMailAbstractMimePart::InlineDisposition
 * The part is presented *inline* in the mail message. For example if you
 * want to display an image attachment directly in the mail, use this.
 *
 * \var QsrMailAbstractMimePart::AttachmentDisposition
 * The part is presented as an *attachment*. This usually means that the user
 * can click and save it to disk.
 */

#include "qsrmailabstractmimepart.h"
#include "qsrmailabstractpart_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \internal
 *
 * \copydoc QsrMailAbstractPart::QsrMailAbstractPart(QsrMailAbstractPart *)
 */
QsrMailAbstractMimePart::QsrMailAbstractMimePart(QsrMailAbstractPartPrivate *dd) :
    QsrMailAbstractPart(dd)
{
}

/*!
 * \copydoc QsrMailAbstractPart::QsrMailAbstractPart(const QsrMailAbstractPart &)
 */
QsrMailAbstractMimePart::QsrMailAbstractMimePart(const QsrMailAbstractMimePart &other) :
    QsrMailAbstractPart(other)
{
}

/*!
 * \copydoc QsrMailAbstractPart::operator=(const QsrMailAbstractPart &)
 */
QsrMailAbstractMimePart &
QsrMailAbstractMimePart::operator=(const QsrMailAbstractMimePart &other)
{
    if (d != other.d) {
        QsrMailAbstractMimePart tmp(other);
        tmp.swap(*this);
    }

    return *this;
}

/*!
 * Swap this instance with *other*. This function is very fast and
 * never fails.
 */
void QsrMailAbstractMimePart::swap(QsrMailAbstractMimePart &other)
{
    qSwap(d, other.d);
}

/*!
 * Destroy the instance.
 */
QsrMailAbstractMimePart::~QsrMailAbstractMimePart()
{
}

/*!
 * Sets an unprocessed header named *name* with the value *value* in the
 * list of headers. If the header already exists it will be replaced. The
 * header is written directly to the wire as part of the message headers
 * without further processing. The header itself must confirm to RFC
 * standards.
 */
void QsrMailAbstractMimePart::setRawHeader(const QByteArray &name,
                                           const QByteArray &value)
{
    d->headers.setHeader(name,value);
}

/*!
 * Append an unprocessed header named *name* with the value *value* to the
 * list of headers for this message.
 */
void QsrMailAbstractMimePart::appendRawHeader(const QByteArray &name,
                                              const QByteArray &value)
{
    d->headers.appendHeader(name, value);
}

/*!
 * Return the value of the first header named *name*. If no header is found
 * a null QByteArray is returned.
 */
QByteArray QsrMailAbstractMimePart::rawHeader(const QByteArray &name) const
{
    return d->headers.value(name);
}

/*!
 * Return a list of values for all headers named *name*.
 */
QList<QByteArray>
QsrMailAbstractMimePart::rawHeaders(const QByteArray &name) const
{
    return d->headers.values(name);
}

/*!
 * Sets the content type of the part to *type*. The default behaviour is
 * to autodetect the content type using the QMimeDatabase while sending the
 * message to the server, so setting this property is optional.
 *
 * However, if the developer decides to override the default behaviour it
 * must be ensured that the supplied content type data confirms to RFC
 * standards.
 *
 * \sa [RFC2045 section 5](http://tools.ietf.org/html/rfc2045#section-5)
 */
void QsrMailAbstractMimePart::setContentType(const QByteArray &type)
{
    d->contentType = type;
}

/*!
 * Return the content type of the part. Be aware that automatic content type
 * detection is only performed while sending the message, so this does return
 * the property as it has been set using setContentType().
 */
QByteArray QsrMailAbstractMimePart::contentType() const
{
    return d->contentType;
}

/*!
 * Sets the content id for this part to *id*. The content id is similar to the
 * message id and is syntactically identical. The content id must be world
 * unique since it is subject to caching, especially if used in
 * message/external bodies. Though the content id is optional it must be
 * supplied for message/external bodies.
 *
 * \sa [RFC2045 section 7](http://tools.ietf.org/html/rfc2045#section-7)
 */
void QsrMailAbstractMimePart::setContentId(const QByteArray &id)
{
    d->contentId = id;
}

/*!
 * Return the content id of the part.
 */
QByteArray QsrMailAbstractMimePart::contentId() const
{
    return d->contentId;
}

/*!
 * Defines the transfer *encoding* used for the part. If no transfer encoding
 * is set it defaults to 7bit encoding. In general you should not need to
 * select an encoding since the implementation of QsrMailMimePart always uses
 * 7bit compatible encodings. However if you should choose to encode the part's
 * body yourself by setting the QsrMailMimePart properties accordingly you must
 * select the appropriate encoding.
 *
 * \sa [RFC2045 section 6](http://tools.ietf.org/html/rfc2045#section-6)
 */
void QsrMailAbstractMimePart::setContentEncoding(const QByteArray &encoding)
{
    d->contentEncoding = encoding;
}

/*!
 * Return the content encoding of the part. Be aware that encoding detection
 * is only performed while sending the message, so this does return the
 * property as it has been set using setContentEncoding().
 */
QByteArray QsrMailAbstractMimePart::contentEncoding() const
{
    return d->contentEncoding;
}

/*!
 * Sets the *description* of this part. The description is a descriptive text
 * for the content of this part compund. This field is optional.
 *
 * \sa [RFC2045 section 8](http://tools.ietf.org/html/rfc2045#section-8)
 */
void QsrMailAbstractMimePart::setContentDescription(const QString &description)
{
    d->contentDescription = description;
}

/*!
 * Returns the content description of the part.
 */
QString QsrMailAbstractMimePart::contentDescription() const
{
    return d->contentDescription;
}

/*!
 * Sets the content disposition for the part. The disposition describes if
 * the part is to be displayed inline or as attachment. The disposition type
 * defaults to InlineDisposition.
 *
 * \sa
 * DispositionType
 */
void QsrMailAbstractMimePart::setContentDisposition(DispositionType type)
{
    d->dispositionType = type;
}

/*!
 * Return the currently set disposition type for this part.
 */
QsrMailAbstractMimePart::DispositionType
QsrMailAbstractMimePart::contentDisposition() const
{
    return d->dispositionType;
}

/*!
 * Defines the filename attribute for this part. This is usually used for
 * parts of the disposition type attachment to signal the MUA the attachments
 * filename. The filename is totally optional. The value for this type must
 * be convertable to QString.
 */
void QsrMailAbstractMimePart::setFilename(const QString &name)
{
    d->filename = name;
}

/*!
 * Returns the filename for the part.
 */
QString QsrMailAbstractMimePart::filename() const
{
    return d->filename;
}

/*!
 * Defines the creation date of this part. The creation date is optional.
 * The value for this type must be convertable to QDateTime.
 */
void QsrMailAbstractMimePart::setCreateDate(const QDateTime &date)
{
    d->createDate = date;
}

/*!
 * Returns the creation date for the part.
 */
QDateTime QsrMailAbstractMimePart::createDate() const
{
    return d->createDate;
}

/*!
 * Defines the modification date of this part. The modification date is
 * optional. The value for this type must be convertable to QDateTime.
 */
void QsrMailAbstractMimePart::setModificationDate(const QDateTime &date)
{
    d->modificationDate = date;
}

/*!
 * Returns the modification date for the part.
 */
QDateTime QsrMailAbstractMimePart::modificationDate() const
{
    return d->modificationDate;
}

/*!
 * Defines the read date of this part. The read date is optional. The value
 * for this type must be convertable to QDateTime.
 */
void QsrMailAbstractMimePart::setReadDate(const QDateTime &date)
{
    d->readDate = date;
}

/*!
 * Returns the read date for the part.
 */
QDateTime QsrMailAbstractMimePart::readDate() const
{
    return d->readDate;
}

/*!
 * Defines the size of the part. If no size is specified the system tries to
 * detect the size automatically while sending the mail. The value for this
 * type must be convertable to int.
 */
void QsrMailAbstractMimePart::setSize(qint64 bytes)
{
    d->size = bytes;
}

/*!
 * Return the size of the part. Since autodetection takes place while sending
 * this will only return the value as it has been set by setSize().
 */
qint64 QsrMailAbstractMimePart::size() const
{
    return d->size;
}

QT_END_NAMESPACE
