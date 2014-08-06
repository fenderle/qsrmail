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
 * \class QsrMailAbstractPart "qsrmailabstractpart.h" <QsrMailAbstractPart>
 * \brief Interface class for all QsrMail*Part implementations.
 */

/*!
 * \internal
 *
 * \class QsrMailAbstractPartPrivate "qsrmailabstractpart_p.h"
 * \brief This is the implementation class for the several message parts.
 *
 * Since the parts have many similarities it is useful to combine all parts data
 * into one shared data object to speed up processing and make handling of the
 * different part types easier for the data renderer.
 *
 * While the user always interacts with the interface implementations to this
 * class (QsrMailBodyPart, QsrMailMimePart or QsrMailMimeMultipart), the
 * renderer only interfaces to this interface and automatically has access
 * to all properties. So this class is something like a union of all required
 * data members for all the implementations including some helper methods eg
 * to render headers.
 *
 * Also it is clear that there is no intention that the user actually implements
 * a QsrMail...Part derivate itself; this is not the design goal.
 */

/*!
 * \enum QsrMailAbstractPart::PartType
 * Selects for which part implementation the data class holds the data.
 *
 * \var QsrMailAbstractPart::NullType
 * This type is used for default constructed values.
 *
 * \var QsrMailAbstractPart::BodyPartType
 * The interface class is QsrMailBodyPart
 *
 * \var QsrMailAbstractPart::MimePartType
 * The interface class is QsrMailMimePart
 *
 * \var QsrMailAbstractPart::MimeMultipartType
 * The interface class is QsrMailMimeMultipart
 *
 */

#include "qsrmailabstractpart.h"
#include "qsrmailabstractpart_p.h"
#include "qsrmailrfctools_p.h"

#include "qsrmailbodypart.h"
#include "qsrmailmimepart.h"
#include "qsrmailmimemultipart.h"

#include <QStringBuilder>
#include <QUuid>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QSharedDataPointer<QsrMailAbstractPartPrivate>,
                          sharedNull, (new QsrMailAbstractPartPrivate(
                                       QsrMailAbstractPart::NullType)));

/*!
 * \internal
 *
 * Creates a new instance of the data class. *type* specifies for which
 * implementation the data class will be used.
 */
QsrMailAbstractPartPrivate::QsrMailAbstractPartPrivate(
        QsrMailAbstractPart::PartType type) :
    partType(type),
    bodyDevice(0),
    autoDelete(false),
    multipartContentType(QsrMailMimeMultipart::MixedType),
    dispositionType(QsrMailAbstractMimePart::InlineDisposition),
    size(0),
    encoder(QsrMailMimePart::AutoDetectEncoder),
    boundary(QUuid::createUuid().toRfc4122().toHex())
{
}

/*!
 * \internal
 *
 * Creates all headers for the data in the implementation according to
 * the type of the frontend class. The generated headers are used by the
 * renderer and sent to the server.
 */
QsrMailHeaders QsrMailAbstractPartPrivate::cookHeaders() const
{
    QsrMailHeaders result(headers);

    QByteArray ctype(contentType);
    if (isMimeMultipart()) {
        switch (multipartContentType) {
        case QsrMailMimeMultipart::MixedType:
            ctype = "multipart/mixed";
            break;
        case QsrMailMimeMultipart::AlternativeType:
            ctype = "multipart/alternative";
            break;
        case QsrMailMimeMultipart::DigestType:
            ctype = "multipart/digest";
            break;
        case QsrMailMimeMultipart::ParallelType:
            ctype = "multipart/parallel";
            break;
        }

        ctype += "; boundary=\"" % boundary % "\"";
    }

    if (!ctype.isEmpty())
        result.setHeader("Content-Type", ctype);

    if (!contentId.isEmpty())
        result.setHeader("Content-ID", contentId);

    if (!contentEncoding.isEmpty())
        result.setHeader("Content-transfer-encoding", contentEncoding);

    if (!contentDescription.isEmpty()) {
        result.setHeader("Content-Description",
                         QsrMailRfcTools::toEncodedWords(contentDescription));
    }

    /* form a RFC2183 Content-Disposition */
    QByteArray disposition;
    switch (dispositionType) {
    case QsrMailAbstractMimePart::InlineDisposition:
        disposition = "inline";
        break;
    case QsrMailAbstractMimePart::AttachmentDisposition:
        disposition = "attachment";
        break;
    }

    if (!createDate.isNull()) {
        disposition += ";\n\tcreation-date=\""
                % QsrMailRfcTools::rfc2822Date(createDate)
                % "\"";
    }

    if (!modificationDate.isNull()) {
        disposition += ";\n\tmodification-date=\""
                % QsrMailRfcTools::rfc2822Date(modificationDate)
                % "\"";
    }

    if (!readDate.isNull()) {
        disposition += ";\n\tread-date=\""
                % QsrMailRfcTools::rfc2822Date(readDate)
                % "\"";
    }

    if (size > 0) {
        disposition += ";\n\tsize="
                % QByteArray::number(size);
    }

    if (!filename.isEmpty()) {
        disposition += ";\n\tfilename*=\"utf-8''"
                % filename.toUtf8().toPercentEncoding()
                % "\"";
    }

    result.setHeader("Content-Disposition", disposition);

    return result;
}

/*!
 * \internal
 *
 * Implementation of the QsrMailAbstractPart::isEmpty() interface method.
 */
bool QsrMailAbstractPartPrivate::isEmptyImpl() const
{
    switch (partType) {
    case QsrMailAbstractPart::BodyPartType:
        return body.isEmpty()
                && bodyDevice == 0;

    case QsrMailAbstractPart::MimePartType:
        return body.isEmpty()
                && bodyDevice == 0
                && headers.headers.isEmpty();

    case QsrMailAbstractPart::MimeMultipartType:
        return headers.headers.isEmpty()
                && parts.isEmpty();

    case QsrMailAbstractPart::NullType:
        break;
    }

    return true;
}

/*!
 * \internal
 *
 * \fn QsrMailAbstractPartPrivate::isBodyPart()
 * Returns true if the part is a body part.
 */

/*!
 * \internal
 *
 * \fn QsrMailAbstractPartPrivate::isMimePart()
 * Returns true if the part is a mime part.
 */

/*!
 * \internal
 *
 * \fn QsrMailAbstractPartPrivate::isMimeMultipart()
 * Returns true if the part is a mime multipart.
 */

/* -------------------------------------------------------------------------- */

/*!
 * \internal
 *
 * Construct a default constructed instance. This is required to be able
 * to have the QsrMailAbstractPart as a member variable
 */
QsrMailAbstractPart::QsrMailAbstractPart() :
    d(*sharedNull)
{
}

/*!
 * \internal
 *
 * Construct a new instance of this class. This class is not designed to be
 * used directly or to be inherited into custom classes. Use the interface
 * implementations instead.
 *
 * \sa
 * QsrMailBodyPart, QsrMailMimePart, QsrMailMimeMultipart
 */
QsrMailAbstractPart::QsrMailAbstractPart(QsrMailAbstractPartPrivate *dd) :
    d(dd)
{
}

/*!
 * Construct a copy of *other*. This operation is fast and takes constant
 * time since object is implicitly shared. Write operations on the shared
 * data detaches it (copy-on-write).
 */
QsrMailAbstractPart::QsrMailAbstractPart(const QsrMailAbstractPart &other) :
    d(other.d)
{
}

/*!
 * Assigns *other* to this QsrMailAbstractPart and returns a pointer to this
 * instance.
 */
QsrMailAbstractPart &
QsrMailAbstractPart::operator=(const QsrMailAbstractPart &other)
{
    if (d != other.d) {
        QsrMailAbstractPart tmp(other);
        tmp.swap(*this);
    }

    return *this;
}

/*!
 * Swap this instance with *other*. This function is very fast and
 * never fails.
 */
void QsrMailAbstractPart::swap(QsrMailAbstractPart &other)
{
    qSwap(d, other.d);
}

/*!
 * Destroy the instance.
 */
QsrMailAbstractPart::~QsrMailAbstractPart()
{
}

/*!
 * Return the type of this part. The type can be used to determine the
 * interface class in case only the abstract class is available. The type
 * can be BodyPartType (QsrMailBodyPart), MimePartType (QsrMailMimePart) or
 * MimeMultipartType (QsrMailMimeMultipart).
 */
QsrMailAbstractPart::PartType QsrMailAbstractPart::type() const
{
    return d->partType;
}

/*!
 * Returns true if the class can be converted to the specified implementation.
 */
bool QsrMailAbstractPart::canConvert(PartType conversion) const
{
    return d->partType == conversion;
}

/*!
 * Convert the data of this class to QsrMailBodyPart. If a conversion is not
 * possible a default constructed object is returned. Make sure the object is
 * convertable using type() or canConvert().
 */
QsrMailBodyPart QsrMailAbstractPart::toBodyPart() const
{
    if (d->partType != BodyPartType)
        return QsrMailBodyPart();

    return *(reinterpret_cast<const QsrMailBodyPart *>(this));
}

/*!
 * Convert the data of this class to QsrMailMimePart. If a conversion is not
 * possible a default constructed object is returned. Make sure the object is
 * convertable using type() or canConvert().
 */
QsrMailMimePart QsrMailAbstractPart::toMimePart() const
{
    if (d->partType != MimePartType)
        return QsrMailMimePart();

    return *(reinterpret_cast<const QsrMailMimePart *>(this));
}

/*!
 * Convert the data of this class to QsrMailMimeMultipart. If a conversion is
 * not possible a default constructed object is returned. Make sure the object
 * is convertable using type() or canConvert().
 */
QsrMailMimeMultipart QsrMailAbstractPart::toMimeMultipart() const
{
    if (d->partType != MimeMultipartType)
        return QsrMailMimeMultipart();

    return *(reinterpret_cast<const QsrMailMimeMultipart *>(this));
}

/*!
 * Return true if this instance contains no data. Only default constructed
 * instances are null. A non-null instance must not be a valid instance.
 */
bool QsrMailAbstractPart::isNull() const
{
    return d == *sharedNull;
}

/*!
 * Returns true if the object is considered to contain no data.
 */
bool QsrMailAbstractPart::isEmpty() const
{
    return d->isEmptyImpl();
}

QT_END_NAMESPACE
