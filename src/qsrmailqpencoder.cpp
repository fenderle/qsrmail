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
 * \class QsrMailQpEncoder qsrmailqpencoder.h <QsrMailQpEncoder>
 * \brief This class provides a quoted printable encoder.
 *
 * The encoder is implemented as a QIODevice wrapper. As such it uses an
 * underlying QIODevice as its data source. All data passing the encoder will be
 * transformed into quoted printable encoding 'in-stream' without allocating
 * huge amounts of buffers.
 *
 * The wrapper will automatically open the underlying device if this should
 * be necessarry (in which case it also will close it automatically). If the
 * device is already open it will be just used. Note that the underlying
 * device must provide QIODevice::ReadOnly access. Also note that the
 * wrapper only supports *reading* from the underlying device.
 *
 * The encoder works in a internet mail compatible fashion meaning it breaks
 * the lines automatically at 76 chars and uses CRLF sequences as linebreaks.
 * The width can be adjusted using setLineWidth().
 *
 * It also takes care of the RFC requirements for soft linebreaks, etc as
 * specified in RFC2045 section 6.7.
 *
 * Example:
 * \code{.cpp}
 * QFile input("inputfile");
 *
 * // assign QFile as parent so the encoder gets destroyed with the source
 * QsrMailQpEncoder *encoder = new QsrMailQpEncoder(&input, &input);
 * if (encoder->open(QIODevice::ReadOnly)) {
 *     qDebug() << encoder->readAll();
 *     encoder->close();
 * }
 * \endcode
 */

/*!
 * \internal
 *
 * \class QsrMailQpEncoderPrivate qsrmailqpencoder_p.h
 * \brief The private data class of the QsrMailTransaction class.
 */

#include "qsrmailqpencoder.h"
#include "qsrmailqpencoder_p.h"

QT_BEGIN_NAMESPACE

/* dictionary for quoted printable encoder */
const char qpdict[] = "0123456789ABCDEF";

/*!
 * \internal
 *
 * Construct a data class for QsrMailQpEncoder from *qq*.
 */
QsrMailQpEncoderPrivate::QsrMailQpEncoderPrivate(QsrMailQpEncoder *qq) :
    QsrMailAbstractEncoderPrivate(qq),
    lineWidth(76),
    lineChars(0)
{
}

/*!
 * \internal
 *
 * Destroy the instance.
 */
QsrMailQpEncoderPrivate::~QsrMailQpEncoderPrivate()
{
}

/*!
 * \internal
 *
 * Implementation for QIODevice::readData(). The method encodes the data
 * on-the-fly to it's quoted printable representation.
 *
 * The data is read from the underlying QIODevice and the converted
 * data is written to *data* up to *maxlen* bytes. The method returns
 * the actual number of bytes written to *data* or -1 if some error
 * occured.
 */
qint64 QsrMailQpEncoderPrivate::readDataImpl(char *data, qint64 maxlen)
{
    Q_Q(QsrMailQpEncoder);

    #define ENSURE_SPACE(n) { \
        if (remaining < n) { \
            device->ungetChar(static_cast<char>(c)); \
            break; \
        } \
    }

    /* multiple readChannelFinished() signals */
    if (deviceAtEnd())
        return 0;

    char c = 0;
    qint64 remaining = maxlen;
    bool isTextModeEnabled = q->isTextModeEnabled();

    while (device->bytesAvailable() && remaining > 0) {
        if (!device->getChar(&c))
            break;

        bool forceEncoding = false;

        /* Rule 3: TAB/SPC followed by linebreak must be encoded */
        if (c == 9 || c == 32) {
            char buf[2];
            if (device->peek(buf, sizeof(buf)) != sizeof(buf)) {
                device->ungetChar(c);
                break;
            }

            forceEncoding = buf[0] == '\r' && buf[1] == '\n';
        }

        /* Rule 4: Linebreaks */
        if (c == '\r') {
            if (!device->getChar(&c)) {
                device->ungetChar('\r');
                break;
            }

            if (c == '\n') {
                ENSURE_SPACE(2)
                *data++ = '\r';
                *data++ = '\n';
                remaining -= 2;
                lineChars = 0;
                continue;
            }

            device->ungetChar(c);
        }

        if (isTextModeEnabled && c == '\n') {
            ENSURE_SPACE(2)
            *data++ = '\r';
            *data++ = '\n';
            remaining -= 2;
            lineChars = 0;
            continue;
        }

        /* make sure . is encoded if it's first char of the line to avoid
         * breaking the mail transfer
         */
        if (lineChars == 0 && c == '.')
            forceEncoding = true;

        /* see if char is printable */
        bool isPrintable = !forceEncoding &&
                ((c >= 33 && c <= 60) || (c >= 62 && c <= 126) ||
                 c == 9 || c == 32);

        /* check if the line can accomodate the output. include space
         * for a '=' in case a linebreak is needed
         */
        if ((lineChars + (isPrintable ? 2 : 4)) >= lineWidth) {
            ENSURE_SPACE(3)
            *data++ = '=';
            *data++ = '\r';
            *data++ = '\n';
            remaining -= 3;
            lineChars = 0;
        }

        /* now write the data itself */
        if (isPrintable) {
            *data++ = c;
            remaining--;
            lineChars++;
        } else {
            ENSURE_SPACE(3)
            *data++ = '=';
            *data++ = qpdict[(static_cast<quint8>(c) >> 4) & 0x0f];
            *data++ = qpdict[static_cast<quint8>(c) & 0x0f];
            remaining -= 3;
            lineChars += 3;
        }
    }

    /* emit signal if at end of input stream */
    if (deviceAtEnd())
        emit q->readChannelFinished();

    return maxlen - remaining;
}

/* -------------------------------------------------------------------------- */

/*!
 * Construct a new quoted printable encoder. The encoder will read it's data
 * from *device* and be a child object of the optional *parent*. Ideally the
 * device is not yet opened.
 */
QsrMailQpEncoder::QsrMailQpEncoder(QIODevice *device, QObject *parent) :
    QsrMailAbstractEncoder(*new QsrMailQpEncoderPrivate(this), device, parent)
{
    Q_D(QsrMailQpEncoder);
    d->device = device;

    /* relay readyRead signal */
    connect(device, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
}

/*!
 * Destroy the encoder instance. This destructor does not call close(),
 * neither on itself, nor on the underlying device. It actually does
 * nothing but freeing its resources. You are encouraged to call close()
 * yourself and properly cleanup after yourself.
 */
QsrMailQpEncoder::~QsrMailQpEncoder()
{
}

/*!
 * Return the number of bytes that are available for reading. This method is
 * not 100% exact since encoding to quoted printable is performed on the fly
 * and the result is not a forseeable function of the input. The return value
 * is the accumulated bytes in the input buffers.
 */
qint64 QsrMailQpEncoder::bytesAvailable() const
{
    Q_D(const QsrMailQpEncoder);

    /* this is not exactly 'right' but good enough; actually there could be
     * more bytes available, but this is the safe assumption
     */
    return d->device->bytesAvailable()
            + QIODevice::bytesAvailable();
}

/*!
 * Returns true if this device is sequential. This device can only be
 * sequential and thus always returns *true*.
 */
bool QsrMailQpEncoder::isSequential() const
{
    return true;
}

/*!
 * Opens the device and sets its OpenMode to *mode*. Returns *true* on
 * success; otherwise *false*. This call also opens the underlying
 * QIODevice if it is not opened yet. If the underlying QIODevice cannot
 * be opened or has been opened with incompatible OpenModes the call also
 * returns *false*.
 *
 * The call only supports QIODevice::ReadOnly access modes since write
 * access is not possible with this implementation.
 */
bool QsrMailQpEncoder::open(OpenMode mode)
{
    Q_D(QsrMailQpEncoder);

    /* avoid dupe calls to prevent signal mess */
    if (isOpen()) {
        setErrorString(tr("device already opened"));
        return false;
    }

    /* accept only read, no write */
    if (mode & QIODevice::WriteOnly) {
        setErrorString(tr("requested OpenMode not supported"));
        return false;
    }

    /* must be buffered (wie make use of ungetChar */
    if (mode & QIODevice::Unbuffered) {
        setErrorString(tr("device must be buffered"));
        return false;
    }

    /* open the underlying QIODevice as requested */
    bool deviceOk = false;
    if (d->device->isOpen()) {
        deviceOk = d->device->openMode() & mode;
    } else {
        deviceOk = d->device->open(mode);
    }
    if (!deviceOk) {
        setErrorString(tr("underlying QIODevice is not accessible"));
        return false;
    }

    /* reset internal states */
    d->lineChars = 0;

    /* call parent implementation (never fails) */
    return QIODevice::open(mode);
}

/*!
 * Set the current position in the file. Since this implementation only
 * supports sequential access this function has no effect and always
 * returns *false*.
 */
bool QsrMailQpEncoder::seek(qint64 pos)
{
    Q_UNUSED(pos)
    return false;
}

/*!
 * Set the output width of a line. The width is specified excluding the
 * CRLF characters at the end of a line. This method is mainly useful
 * if the encoder is used in the context of creating MIME bodies for
 * internet mail which are constraint to 76 charcters per line so they
 * do not exceed the SMTP specification.
 *
 * Passing zero as *width* disables line wrapping.
 */
void QsrMailQpEncoder::setLineWidth(int value)
{
    Q_D(QsrMailQpEncoder);
    d->lineWidth = value;
}

/*!
 * Returns the current line width in characters, excluding the CRLF at
 * the end of the line.
 */
int QsrMailQpEncoder::lineWidth() const
{
    Q_D(const QsrMailQpEncoder);
    return d->lineWidth;
}

/*!
 * Reimplemented for subclassing QIODevice.
 */
qint64 QsrMailQpEncoder::readData(char *data, qint64 maxlen)
{
    Q_D(QsrMailQpEncoder);
    return d->readDataImpl(data, maxlen);
}

/*!
 * Reimplemented for subclassing QIODevice.
 */
qint64 QsrMailQpEncoder::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)

    return -1;
}

QT_END_NAMESPACE

#include "moc_qsrmailqpencoder.cpp"
