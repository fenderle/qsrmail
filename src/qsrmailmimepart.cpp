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
 * \class QsrMailMimePart qsrmailmimepart.h <QsrMailMimePart>
 * \brief This class defines a mime part of a mime multipart message.
 *
 * The part has to be appended to a multipart compound. Please note that the
 * QsrMailMessage class provides a simple interface to append a single part
 * without having to create the whole multipart structure by yourself.
 *
 * As with QsrMailMessage it is possible to inject raw headers into the
 * multipart compound. The same handling rules apply here.
 *
 * \sa QsrMailMessage, QsrMailMimeMultipart
 */

/*!
 * \enum QsrMailMimePart::Encoder
 * The enum represents the different possible ncoder types.
 *
 * \var QsrMailMimePart::AutoDetectEncoder
 * Select a suitable encoder, based on the content type. This results in
 * QuotedPrintableEncoder for text/... content types and Base64Encoder
 * for everything else.
 *
 * \var QsrMailMimePart::PassthroughEncoder
 * Do nothing - just pass the data
 *
 * \var QsrMailMimePart::QuotedPrintableEncoder
 * The data is encoded using the QsrMailQpEncoder which results in quoted
 * printable encoded data.
 *
 * \var QsrMailMimePart::Base64Encoder
 * The data is encoded using the QsrMailBase64Encoder which results in base64
 * encoded data.
 */

#include "qsrmailmimepart.h"
#include "qsrmailabstractpart_p.h"

#include <QFileDevice>
#include <QFileInfo>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QSharedDataPointer<QsrMailAbstractPartPrivate>,
                          sharedNull, (new QsrMailAbstractPartPrivate(
                                       QsrMailAbstractPart::MimePartType)));

/*!
 * Constructs an empty mime part.
 */
QsrMailMimePart::QsrMailMimePart() :
    QsrMailAbstractMimePart(*sharedNull)
{
}

/*!
 * \copydoc QsrMailAbstractPart::QsrMailAbstractPart(const QsrMailAbstractPart &)
 */
QsrMailMimePart::QsrMailMimePart(const QsrMailMimePart &other) :
    QsrMailAbstractMimePart(other)
{
}

/*!
 * \copydoc QsrMailAbstractPart::operator=(const QsrMailAbstractPart &)
 */
QsrMailMimePart &QsrMailMimePart::operator=(const QsrMailMimePart &other)
{
    if (d != other.d) {
        QsrMailMimePart tmp(other);
        tmp.swap(*this);
    }

    return *this;
}

/*!
 * Swap this instance with *other*. This function is very fast and
 * never fails.
 */
void QsrMailMimePart::swap(QsrMailMimePart &other)
{
    qSwap(d, other.d);
}

/*!
 * Destroy the instance.
 */
QsrMailMimePart::~QsrMailMimePart()
{
}

/*!
 * \copydoc QsrMailAbstractPart::isNull()
 */
bool QsrMailMimePart::isNull() const
{
    return d == *sharedNull;
}

/*!
 * \copydoc QsrMailBodyPart::setAutoDelete(bool)
 */
void QsrMailMimePart::setAutoDelete(bool enabled)
{
    d->autoDelete = enabled;
}

/*!
 * \copydoc QsrMailBodyPart::autoDelete()
 */
bool QsrMailMimePart::autoDelete() const
{
    return d->autoDelete;
}

/*!
 * Selects the *encoder* used to prepare the data for transfer. The default
 * behaviour is to autodetect the encoder. For text/... content types the
 * quoted printable encoder is selected to ensure human readability of the
 * text. All other content types select the base64 encoder to make the
 * transport binary safe. Both encoders ensure 7bit safe output which
 * usually passes all MTAs without problems.
 *
 * If for any reason the developer wants to self-encode the data the
 * passthrough encoder can be chosen to skip the encoding entirely.
 *
 * \note
 * Any encoder other than the passthrough encoder **overrides** the content
 * encoding type.
 *
 * \sa setContentEncoding()
 */
void QsrMailMimePart::setContentEncoder(Encoder encoder)
{
    d->encoder = encoder;
    d->contentEncoding = QByteArray();
}

/*!
 * Returns the selected content encoder.
 */
QsrMailMimePart::Encoder QsrMailMimePart::contentEncoder() const
{
    return d->encoder;
}

/*!
 * \copydoc QsrMailAbstractMimePart::setContentEncoding(const QByteArray &)
 *
 * \note
 * Setting the content encoding automatically selects the passthrough
 * encoder
 *
 * \sa setContentEncoder()
 */
void QsrMailMimePart::setContentEncoding(const QByteArray &encoding)
{
    d->contentEncoding = encoding;
    d->encoder = PassthroughEncoder;
}

/*!
 * \copydoc QsrMailBodyPart::setBody(const QByteArray &)
 */
void QsrMailMimePart::setBody(const QByteArray &content)
{
    d->body = content;
}

/*!
 * \copydoc QsrMailBodyPart::body()
 */
QByteArray QsrMailMimePart::body() const
{
    return d->body;
}

/*!
 * \copydoc QsrMailBodyPart::setBodyDevice(QIODevice *)
 */
void QsrMailMimePart::setBodyDevice(QIODevice *device)
{
    d->bodyDevice = device;
}

/*!
 * \copydoc QsrMailBodyPart::bodyDevice()
 */
QIODevice *QsrMailMimePart::bodyDevice() const
{
    return d->bodyDevice;
}

/*!
 * Construct a part from a file *device*. The resulting QsrMailMimePart object
 * is an attachment with the contents of the file. The filename attribute
 * will be set to the basename of the file associated with *device*. The
 * object takes ownership of the device and disposes it after it has been
 * processed.
 */
QsrMailMimePart QsrMailMimePart::fromFile(QFileDevice *device)
{
    QsrMailMimePart result;
    QFileInfo info(device->fileName());

    result.d->bodyDevice = device;
    result.d->dispositionType = AttachmentDisposition;
    result.d->filename = info.fileName();
    result.d->autoDelete = true;

    return result;
}

/*!
 * Construct a part from a *data* block. The resulting QsrMailMimePart
 * object is an attachment with the contents of *data*. The filename
 * attribute will be set to the basename of *filename* and the size of
 * *data* is used for the size attribute.
 */
QsrMailMimePart QsrMailMimePart::fromRawData(const QString &filename,
                                             const QByteArray &data)
{
    QsrMailMimePart result;
    QFileInfo info(filename);

    result.d->body = data;
    result.d->dispositionType = AttachmentDisposition;
    result.d->filename = info.fileName();
    result.d->size = data.size();

    return result;
}

/*!
 * Construct a part from a *device*. The resulting QsrMailMimePart object
 * is an attachment with the contents of the data found in the *device*.
 * The filename attribute will be set to the basename of *filename*.
 *
 * \note
 * The ownership of *device* remains with the caller.
 */
QsrMailMimePart QsrMailMimePart::fromDevice(const QString &filename,
                                            QIODevice *device)
{
    QsrMailMimePart result;
    QFileInfo info(filename);

    result.d->bodyDevice = device;
    result.d->dispositionType = AttachmentDisposition;
    result.d->filename = info.fileName();

    return result;
}

/*!
 * Construct a part from a simple *text*. The resulting QsrMailMimePart
 * is an inline attachment of text/plain with UTF-8 encoding, suited for
 * text elements within a mail.
 */
QsrMailMimePart QsrMailMimePart::fromText(const QString &text)
{
    QsrMailMimePart result;

    result.d->body = text.toUtf8();
    result.d->dispositionType = InlineDisposition;
    result.d->contentType = "text/plain; charset=UTF-8";
    result.d->size = result.d->body.size();

    return result;
}

QT_END_NAMESPACE
