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
 * \class QsrMailRfcTools "qsrmailrfctools_p.h"
 * \brief This is a private helper class which handles the required operations
 * for RFC checks.
 *
 * For example it validates internet mail addresses. The methods are used
 * internally and are not directly exposed to the API.
 */

#include "qsrmailrfctools_p.h"

#include <QString>
#include <QStringBuilder>

QT_BEGIN_NAMESPACE

/* return c if ATEXT, 0 otherwise */
#define ATEXT(c) ((c & 0x80) ? 0 : atext[static_cast<quint8>(c)])
static const char atext[128] = {
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      '!',    0,      '#',    '$',    '%',    '&',    '\'',
    0,      0,      '*',    '+',    0,      '-',    0,      '/',
    '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
    '8',    '9',    0,      0,      0,      '=',    0,      '?',
    0,      'A',    'B',    'C',    'D',    'E',    'F',    'G',
    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    'X',    'Y',    'Z',    0,      0,      0,      '^',    '_',
    '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
    'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
    'x',    'y',    'z',    '{',    '|',    '}',    '~',    0
};

/* return c if DTEXT, 0 otherwise */
#define DTEXT(c) ((c & 0x80) ? 0 : dtext[static_cast<quint8>(c)])
static const char dtext[128] = {
    0,      1,      2,      3,      4,      5,      6,      7,
    8,      0,      0,      11,     12,     0,      14,     15,
    16,     17,     18,     19,     20,     21,     22,     23,
    24,     25,     26,     27,     28,     29,     30,     31,
    0,      '!',    '"',    '#',    '$',    '%',    '&',    '\'',
    '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
    '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
    '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
    '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    'X',    'Y',    'Z',    0,      0,      0,      '^',    '_',
    '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
    'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
    'x',    'y',    'z',    '{',    '|',    '}',    '~',    127
};

/* return c if ENCODEDTEXT, 0 otherwise (RFC2047, Section 5(3)) */
#define ENCODEDTEXT(c) ((c & 0x80) ? 0 : encodedtext[static_cast<quint8>(c)])
static const char encodedtext[128] = {
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      '!',    0,      0,      0,      0,      0,      0,
    0,      0,      '*',    '+',    0,      '-',    0,      '/',
    '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
    '8',    '9',    0,      0,      0,      '=',    0,      0,
    0,      'A',    'B',    'C',    'D',    'E',    'F',    'G',
    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
    'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
    'X',    'Y',    'Z',    0,      0,      0,      0,      0,
    0,      'a',    'b',    'c',    'd',    'e',    'f',    'g',
    'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
    'x',    'y',    'z',    0,      0,      0,      0,      0
};

/* hex table */
#define TOHEX(n) (tohex[n & 0xf])
static const char tohex[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

#define isFWS(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')
#define isATEXT(c) (ATEXT(c) != 0)
#define isDTEXT(c) (DTEXT(c) != 0)
#define isENCODEDTEXT(c) (ENCODEDTEXT(c) != 0)

/* weekdays (1 based offset) */
static const char *wdays[7] = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

/* month names (1 based offset) */
static const char *months[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*!
 * \internal
 *
 * Validate *data* to contain a valid internet mail address according to
 * RFC2822. This only includes the 'address' part and does not include the
 * 'display name' part.
 *
 * Returns true if the data buffer contains a valid internet mail address.
 */
bool QsrMailRfcTools::validateAddrSpec(const QByteArray &data)
{
    /* validate an addr-spec according RFC2822; this impementation
     * is rather strict, but still has some holes.
     */
    const char *c = data.constData();
    char ch = 0;

    /* -- LOCAL PART -- */
    c = skipCFWS(c);
    if (c == 0 || *c == 0)
        return false;

    /* check for quoted string */
    if (*c == '"') {
        /* skip quoted */
        while ((ch = *(++c)) && ch != '"') {
            if (ch & 0x80) {
                return false;
            } else if (ch == '\\' && *(c+1) != 0) {
                c++;
            }
        }

        /* terminated ? */
        if (*c++ != '"')
            return false;
    } else {
        /* if not quoted string it must be a dot-atom */
        while (*c == '.' || isATEXT(*c))
            c++;
    }

    /* skip additional CFWS after local part, then there must
     * follow an @
     */
    c = skipCFWS(c);
    if (c == 0 || *c != '@')
        return false;

    /* -- DOMAIN PART -- */
    c = skipCFWS(++c);
    if (c == 0 || *c == 0)
        return false;

    /* check for domain literal */
    if (*c == '[') {
        /* skip domain literal */
        while ((ch = *(++c)) && ch != ']') {
            if (ch == '\\' && *(c+1) != 0) {
                c++;
            } else if (!isDTEXT(ch) && !isFWS(ch)) {
                break;
            }
        }

        /* terminated ? */
        if (*c++ != ']')
            return false;
    } else {
        /* if not domain literal it must be a dot-atom */
        while (*c == '.' || isATEXT(*c))
            c++;
    }

    /* skip last CFWS, after that the string must end */
    c = skipCFWS(c);
    return c != 0 && *c == 0;
}

/*!
 * \internal
 *
 * Validate *data* to contain a valid display name according to RFC2822. The
 * buffer must not contain an internet mail address and only checks the
 * 'display name' part of an address.
 *
 * Returns true if the data buffer contains a valid display name.
 */
bool QsrMailRfcTools::validateDisplayName(const QByteArray &data)
{
    /* display name can hold atom or a quoted string */
    const char *c = data.constData();
    char ch = 0;

    /* skip initial CFWS */
    c = skipCFWS(c);
    if (c == 0 || *c == 0)
        return false;

    /* check for quoted string */
    if (*c == '"') {
        /* skip quoted */
        while ((ch = *(++c)) && ch != '"') {
            if (ch & 0x80) {
                return false;
            } else if (ch == '\\' && *(c+1) != 0) {
                c++;
            }
        }

        /* terminated ? */
        if (*c++ != '"')
            return false;
    } else {
        /* if not quoted string it must be a dot-atom */
        while (*c == '.' || isATEXT(*c))
            c++;
    }

    /* skip last CFWS, after that the string must end */
    c = skipCFWS(c);
    return c != 0 && *c == 0;
}

/*!
 * \internal
 *
 * Encode *data* to a QByteArray representation as defined by RFC2047
 * section 5 'Encoded Words'. This method is useful when UTF-8 strings
 * should be transfered using internet headers since most header values
 * are allowed to carry encoded words.
 */
QByteArray QsrMailRfcTools::toEncodedWords(const QString &data)
{
    /* this is the smallest denominator for the qp encoding according
     * to RFC2047/Section 5 (3)
     */
    QByteArray text(data.toUtf8());
    QByteArray encoded(text.size() * 3, Qt::Uninitialized);
    const char *s = text.constData();
    char *d = encoded.data();
    char *t = d;
    bool isEncoded = false;

    while (char c = *s++) {
        if (isENCODEDTEXT(c)) {
            *t++ = c;
        } else if (c == ' ') {
            /* encode space as _ */
            *t++ = '_';
            isEncoded = true;
        } else {
            *t++ = '=';
            *t++ = TOHEX((c >> 4) & 0xf);
            *t++ = TOHEX(c & 0xf);
            isEncoded = true;
        }
    }
    encoded.truncate(static_cast<int>(t-d));

    /* if the string has been encoded we need to apply some wrapper to it */
    if (isEncoded)
        return "=?UTF-8?Q?" % encoded % "?=";

    /* the string didn't need any encoding */
    return text;
}

/*!
 * \internal
 *
 * Encode *dateTime* to an RFC2822 compliant date representation.
 * The format looks something like 'Sun, 18 May 2014 15:39:32 +0200' and
 * is suitable for use in internet headers like the Date header.
 */
QByteArray QsrMailRfcTools::rfc2822Date(const QDateTime &dateTime)
{
    /* format a date time string according to RFC2822 */
    int dow = dateTime.date().dayOfWeek() - 1;
    int mon = dateTime.date().month() - 1;

    /* bounds check for array access */
    if (dow < 0 || dow > 6 || mon < 0 || mon > 11)
        return QByteArray();

    /* calculate timezone offset */
    QDateTime utc(dateTime.toUTC());
    QDateTime helper(dateTime);
    helper.setTimeSpec(Qt::UTC);
    qint64 diff = utc.secsTo(helper);
    char sign = '+';
    if (diff < 0) {
        sign = '-';
        diff *= -1;
    }

    /* do some formatting vodoo to accomplish the task - use the qt datetime
     * formatting where possible, but substitute the names by known, not
     * localized values and take care of the timezone stuff
     */
    return dateTime.toString(QLatin1String("%1, dd %2 yyyy hh:mm:ss %3%4%5"))
            .arg(wdays[dow])
            .arg(months[mon])
            .arg(QLatin1Char(sign))
            .arg(diff / 3600, 2, 10, QLatin1Char('0'))
            .arg(diff % 3600 / 60, 2, 10, QLatin1Char('0'))
            .toLatin1();
}

/*!
 * \internal
 *
 * Helper function to skip folded whitespaces (FWS) and comments (C) in
 * a character buffer. The implementation is aware of nested comments. On
 * entry the function expects *c* to point to a whitespace or a '('
 * character. The implementation skips all subsequent characters until the
 * first non whitespace or comment character appears to which the call
 * returns the pointer.
 *
 * On (nesting) errors the function returns null to indicate the error.
 */
const char *QsrMailRfcTools::skipCFWS(const char *c)
{
    int level = 0;

    while (*c) {
        /* skip spaces */
        while (isFWS(*c))
            c++;

        /* done? */
        if (*c != '(')
            break;

        /* skip till ending ), can be nested, skip escaped chars, fail
         * on non USASCII chars
         */
        do {
            if (*c & 0x80) {
                return 0;
            } else if (*c == '(') {
                level++;
            } else if (*c == ')') {
                if (--level <= 0) {
                    c++;
                    break;
                }
            } else if (*c == '\\' && *(c+1) != 0) {
                c++;
            }
        } while(*(++c));
    }

    return (level == 0) ? c : 0;
}

QT_END_NAMESPACE
