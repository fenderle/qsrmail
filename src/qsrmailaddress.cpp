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
 * \class QsrMailAddress qsrmailaddress.h <QsrMailAddress>
 * \brief The QsrMailAdress class is a represenation of an internet email
 * address.
 *
 * Main purpose of this class is to offer an easy way of dealing with
 * email addresses while making sure they are syntactically valid. Thus an
 * valid email address must pass tests which makes it an RFC2822 appropriate
 * internet address.
 *
 * The email address consists of an address part (the actual email address)
 * and an optional display name. A valid address can be converted in it's
 * octet representation using the toByteArray() method. This call will also
 * convert the display name - which is represented as an utf-8 QString - into
 * it's octet representation using the encoded word procedure as described in
 * RFC2047 section 5.
 *
 * Usage example:
 * \code
 * #include <QsrMailAddress>
 * #include <QDebug>
 *
 * int main(int argc, const char *argv[])
 * {
 *     QsrMailAddress address("h.mueller@foo.com");
 *
 *     // both variants output: h.mueller@foo.com
 *     qDebug() << address.toString();
 *     qDebug() << address.toByteArray();
 *
 *     address.setDisplayName("Henry Müller");
 *
 *     // outputs: Henry Müller <h.mueller@foo.com>
 *     qDebug() << address.toString();
 *
 *     // outputs: =?UTF-8?Q?Henry_M=C3=BCller?= <h.mueller@foo.com>
 *     qDebug() << address.toByteArray();
 * }
 * \endcode
 */

/*!
 * \internal
 *
 * \class QsrMailAddressPrivate "qsrmailaddress_p.h"
 * \brief Private data class for QsrMailAddress.
 */

#include "qsrmailaddress.h"
#include "qsrmailaddress_p.h"
#include "qsrmailrfctools_p.h"

#include <QStringBuilder>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QSharedDataPointer<QsrMailAddressPrivate>,
                          sharedNull, (new QsrMailAddressPrivate));

/*!
 * \internal
 *
 * Default constructor.
 */
QsrMailAddressPrivate::QsrMailAddressPrivate()
{
}

/* -------------------------------------------------------------------------- */

/*!
 * Constructs an empty mail address. Empty addresses are null and
 * also not valid.
 */
QsrMailAddress::QsrMailAddress() :
    d(*sharedNull)
{
}

/*!
 * Construct a mail address from *address*. The supplied internet mail
 * address which must consist only of valid ASCII characters although
 * the QString type may suggest differently. The address must also not
 * include the display name.
 *
 * Example:
 * \code
 * QsrMailAddress address("h.mueller@foo.com");
 * \endcode
 */
QsrMailAddress::QsrMailAddress(const QString &address) :
    d(new QsrMailAddressPrivate)
{
    d->address = address;
}

/*!
 * Construct a copy of *other*. This operation is fast and takes constant
 * time since QsrMailAddress is implicitly shared. Write operations on
 * the shared data detaches it (copy-on-write).
 */
QsrMailAddress::QsrMailAddress(const QsrMailAddress &other) :
    d(other.d)
{
}

/*!
 * Assigns *other* to this QsrMailAddress and returns this instance.
 */
QsrMailAddress &QsrMailAddress::operator=(const QsrMailAddress &other)
{    
    if (d != other.d) {
        QsrMailAddress tmp(other);
        tmp.swap(*this);
    }

    return *this;
}

/*!
 * Swap this instance with *other*. This function is very fast and
 * never fails.
 */
void QsrMailAddress::swap(QsrMailAddress &other)
{
    qSwap(d, other.d);
}

/*!
 * Destroys the instance.
 */
QsrMailAddress::~QsrMailAddress()
{
}

/*!
 * Construct a mail address from an internet mail address and a display
 * name. Though the *address* parameter must only consist of valid ASCII
 * characters the *displayName* may contain UTF-8 characters.
 *
 * Example:
 * \code
 * QsrMailAddress address("h.mueller@foo.com", "Henry Müller");
 * \endcode
 */
QsrMailAddress::QsrMailAddress(const QString &address,
                               const QString &displayName) :
    d(new QsrMailAddressPrivate)
{
    d->address = address;
    d->displayName = displayName;
}

/*!
 * Compares this mail address to the *other* instance. Returns *true*
 * if they share the same data.
 */
bool QsrMailAddress::operator==(const QsrMailAddress &other) const
{
    return d == other.d;
}

/*!
 * Compares this mail address to another (internet mail) *address*. This
 * only compares the internet mail address part of the instance. Returns
 * *true* if the address matches.
 */
bool QsrMailAddress::operator==(const QString &address) const
{
    return d->address == address.toUtf8();
}

/*!
 * Return true if this instance contains no data. Only default constructed
 * instances are null. A non-null instance cannot be a valid instance.
 */
bool QsrMailAddress::isNull() const
{
    return d == *sharedNull;
}

/*!
 * Return true if the instance contains valid data. An instance is valid if
 * it contains an RFC compliant internet mail address. A valid instance cannot
 * be null.
 */
bool QsrMailAddress::isValid() const
{
    return QsrMailRfcTools::validateAddrSpec(d->address.toUtf8());
}

/*!
 * Set the internet mail address to *address*. The *address* must contain
 * a valid ASCII sequence as defined in RFC2822. The instance will only be
 * considered valid if the internet mail address complies to the RFC
 * standard.
 *
 * The given address will be set **regardless** of it's validity. Use the
 * isValid() method to check if the address is actually valid.
 */
void QsrMailAddress::setAddress(const QString &address)
{
    d->address = address;
}

/*!
 * Return the currently set internet mail address. The address might be
 * invalid. Use the isValid() method to see if the current address is
 * valid.
 */
QString QsrMailAddress::address() const
{
    return d->address;
}

/*!
 * Set the display name for the address. The display name is the human
 * readable part of an internet mail address. This string may contain
 * UTF-8 data.
 */
void QsrMailAddress::setDisplayName(const QString &name)
{
    d->displayName = name;
}

/*!
 * Return the currently set display name.
 */
QString QsrMailAddress::displayName() const
{
    return d->displayName;
}

/*!
 * Returns the encoded representation of the address and the display name.
 * This version is suitable for internet mail headers. Thus it complies
 * to RFC2822. Display names containing UTF-8 characters will be encoded
 * using RFC2047 encoded words.
 *
 * Example:
 * \code{.cpp}
 * QsrMailAddress email;
 * email.setAddress("h.mueller@foo.com");   // internet mail address
 * email.setDisplayName("Henry Müller");    // (utf-8) display name
 *
 * QByteArray result = email.toByteArray();
 * // result: '=?UTF-8?Q?Henry_M=C3=BCller?= <h.mueller@foo.com>'
 * \endcode
 */
QByteArray QsrMailAddress::toByteArray() const
{
    if (!isValid())
        return QByteArray();

    if (d->displayName.isEmpty())
        return d->address.toUtf8();

    if (QsrMailRfcTools::validateDisplayName(d->displayName.toUtf8()))
        return d->displayName.toUtf8() % " <" % d->address.toUtf8() % ">";

    return QsrMailRfcTools::toEncodedWords(d->displayName)
            % " <" % d->address.toUtf8() % ">";
}

/*!
 * Returns the UTF-8 representation of the address and the display name.
 *
 * Example:
 * \code{.cpp}
 * QsrMailAddress email;
 * email.setAddress("h.mueller@foo.com");   // internet mail address
 * email.setDisplayName("Henry Müller");    // (utf-8) display name
 *
 * QString result = email.toString();
 * // result: 'Henry Müller <h.mueller@foo.com>'
 * \endcode
 */
QString QsrMailAddress::toString() const
{
    if (!isValid())
        return QString();

    if (d->displayName.isEmpty())
        return d->address;

    return d->displayName % " <" % d->address % ">";
}

QT_END_NAMESPACE
