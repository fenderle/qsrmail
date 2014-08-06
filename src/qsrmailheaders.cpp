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
 * \internal
 *
 * \class QsrMailHeaders "qsrmailheaders_p.h"
 * \brief Wrapper class to handle internet headers somewhat more easily.
 *
 * This also provides some helper functions to render the headers directly to a
 * QByteArray buffer.
 */

/*!
 * \internal
 *
 * \typedef QsrMailHeaders::HeaderPair
 * Represents a header name with a header value. The header name an value
 * must be RFC complient encoded.
 */

/*!
 * \internal
 *
 * \typedef QsrMailHeaders::HeaderList
 * A list of headers formed of HeaderPairs
 */

#include "qsrmailheaders_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \internal
 *
 * Set a header *name* with *value* in the list. If the header exists it
 * will be replaced. If the header does not exist it will be appended
 * to the list of headers.
 */
void QsrMailHeaders::setHeader(const QByteArray &name, const QByteArray &value)
{
    /* do nothing on empty names */
    if (name.isEmpty())
        return;

    /* remove all instances of the named header */
    HeaderList::Iterator it(headers.begin());
    while (it != headers.end()) {
        if (it->first == name)
            it = headers.erase(it);
        else
            ++it;
    }

    /* if value is null the user expects a delete */
    if (value.isNull())
        return;

    /* add the new header */
    headers.append(HeaderPair(name, value));
}

/*!
 * \internal
 *
 * Append a header *name* with *value* to the list. The header is always
 * appended regardless if or if not a header named *name* already exists
 * int the list.
 */
void QsrMailHeaders::appendHeader(const QByteArray &name,
                                  const QByteArray &value)
{
    /* do nothing on invalid data */
    if (name.isEmpty() || value.isNull())
        return;

    /* add the new header */
    headers.append(HeaderPair(name, value));
}

/*!
 * \internal
 *
 * Append all headers from the *other* instance to this instance.
 */
void QsrMailHeaders::appendHeader(const QsrMailHeaders &other)
{
    /* append headers from other object */
    headers.append(other.headers);
}

/*!
 * \internal
 *
 * Find a header by its *name* and return an iterator pointing to the
 * first header found in the list. The list is searched from the beginning.
 *
 * If no header name matches *name* a iterator to headers.constEnd() is
 * returned.
 */
QsrMailHeaders::HeaderList::ConstIterator
QsrMailHeaders::findHeader(const QByteArray &name) const
{
    HeaderList::ConstIterator it(headers.constBegin());
    while (it != headers.constEnd()) {
        if (it->first == name)
            break;
        ++it;
    }

    return it;
}

/*!
 * \internal
 *
 * Return *true* if the list contains at least one header named *name*.
 * This is a convenience wrapper for findHeader().
 */
bool QsrMailHeaders::hasHeader(const QByteArray &name) const
{
    return findHeader(name) == headers.constEnd();
}

/*!
 * \internal
 *
 * Return the value of the first header in the list named *name*. If no
 * header is found by this *name* a null QByteArray is returned.
 */
QByteArray QsrMailHeaders::value(const QByteArray &name) const
{
    HeaderList::ConstIterator it(findHeader(name));
    if (it == headers.constEnd())
        return QByteArray();

    return it->second;
}

/*!
 * \internal
 *
 * Return the value of all headers in the list named *name*.
 */
QList<QByteArray> QsrMailHeaders::values(const QByteArray &name) const
{
    QList<QByteArray> result;

    HeaderList::ConstIterator it(headers.constBegin());
    while (it != headers.constEnd()) {
        if (it->first == name)
            result.append(it->second);
    }

    return result;
}

/*!
 * \internal
 *
 * Render all headers into a QByteArray and return the buffer. This is
 * useful for writing the headers to the wire. The implementation
 * iterates the header list, outputs the header name, followd by ': ',
 * followed by the value, followed by CRLF.
 */
QByteArray QsrMailHeaders::renderHeaders() const
{
    QByteArray result;
    HeaderList::ConstIterator it(headers.constBegin());
    while (it != headers.constEnd()) {
        if (!(it->first.isEmpty()) && !(it->second.isNull())) {
            result += it->first;
            result += ": ";
            result += it->second;
            result += "\r\n";
        }

        ++it;
    }

    return result;
}

QT_END_NAMESPACE
