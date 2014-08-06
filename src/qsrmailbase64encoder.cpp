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
 * \class QsrMailBase64Encoder qsrmailbase64encoder.h <QsrMailBase64Encoder>
 * \brief This class provides a Base64 encoder.
 *
 * The encoder is implemented as a QIODevice wrapper. As such it uses an
 * underlying QIODevice as its data source. All data passing the encoder will
 * be transformed into Base64 encoding 'in-stream' without allocating huge
 * amounts of buffers.
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
 * Example:
 * \code{.cpp}
 * QFile input("inputfile");
 *
 * // assign QFile as parent so the encoder gets destroyed with the source
 * QsrMailBase64Encoder *encoder = new QsrMailBase64Encoder(&input, &input);
 * if (encoder->open(QIODevice::ReadOnly)) {
 *     qDebug() << encoder->readAll();
 *     encoder->close();
 * }
 * \endcode
 */

/*!
 * \internal
 *
 * \class QsrMailBase64EncoderPrivate "qsrmailbase64encoder_p.h"
 * \brief Private data class for QsrMailBase64Encoder.
 */

#include "qsrmailbase64encoder.h"
#include "qsrmailbase64encoder_p.h"

QT_BEGIN_NAMESPACE

/* dictionary for base64 encoder */
const char base64dict[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef"
                          "ghijklmnopqrstuvwxyz0123456789+/";

/*!
 * \internal
 *
 * Construct a data class for QsrMailBase64Encoder from *qq*.
 */
QsrMailBase64EncoderPrivate::QsrMailBase64EncoderPrivate(QsrMailBase64Encoder *qq) :
    QsrMailAbstractEncoderPrivate(qq),
    lineWidth(76),
    lineChars(0),
    qBuffer(0),
    qSize(0)
{
}

/*!
 * \internal
 *
 * Destroy the instance.
 */
QsrMailBase64EncoderPrivate::~QsrMailBase64EncoderPrivate()
{
}

/*!
 * \internal
 *
 * Sets a flag which finalizes the buffers during the next readData() and
 * thus flushes the buffers to the output.
 */
void QsrMailBase64EncoderPrivate::_q_flushBuffers()
{
    Q_Q(QsrMailBase64Encoder);

    if (qSize > 0) {
        gotReadChannelFinished = true;
        emit q->readyRead();
    }
}

/*!
 * \internal
 *
 * Implementation for QIODevice::readData(). Here is where the magic
 * happens. This is the engine which converts the data into Base64.
 *
 * The data is read from the underlying QIODevice and the converted
 * data is written to *data* up to *maxlen* bytes. The method returns
 * the actual number of bytes written to *data* or -1 if some error
 * occured.
 *
 * As of now the buffer requires at least six bytes of space for the
 * function to do anything useful.
 */

qint64 QsrMailBase64EncoderPrivate::readDataImpl(char *data, qint64 maxlen)
{
    Q_Q(QsrMailBase64Encoder);

    /* multiple readChannelFinished() signals */
    if (qSize == 0 && deviceAtEnd())
        return 0;

    quint8 c = 0;
    qint64 remain = maxlen;

    /* we need at least 6 bytes (4*BASE64, opt. CR,LF) to be happy */
    while (device->bytesAvailable() > 0 && remain > 6) {
        /* fill quantum */
        while (qSize < 3) {
            if (!device->getChar(reinterpret_cast<char *>(&c)))
                break;

            switch (qSize) {
            case 0:
                qBuffer |= static_cast<quint32>(c << 16 & 0xff0000);
                break;
            case 1:
                qBuffer |= static_cast<quint32>(c << 8 & 0xff00);
                break;

            case 2:
                qBuffer |= static_cast<quint32>(c & 0xff);
                break;
            }

            qSize++;
        }

        /* push quantum to output buffer if accumlated */
        if (qSize == 3)
            remain -= putQ(&data);
    }

    /* handle end of input device */
    if (deviceAtEnd()) {
        /* flush qBuffer.. */
        if (qSize > 0)
            remain -= putQ(&data);

        /* emit signal if at end of input stream */
        emit q->readChannelFinished();
    }

    return maxlen - remain;
}

/*!
 * \internal
 *
 * \fn QsrMailBase64EncoderPrivate::put(char **, char)
 * Helper function to put an encoded character *c* at the position *p*.
 * The function takes care of the linebreak and shifts *p* accordingly.
 * It must be ensured that the buffer to which *p* points is large enough
 * to accompany up to three bytes. If the character is written *p* is shifted.
 *
 * Returns the number of bytes actually written.
 */

/*!
 * \internal
 *
 * \fn QsrMailBase64EncoderPrivate::putQ(char **)
 * Puts the bytes from the buffer at the memory pointed to by *p*. The function
 * always writes 4 bytes to the output. If the buffer does not have enough
 * information the function will pad the result to 4 bytes. After execution
 * *p* is shifted to the next available byte.
 *
 * The function return the number of bytes written.
 */

/* -------------------------------------------------------------------------- */

/*!
 * Construct a new Base64 encoder from *device* and optional *parent*.
 * Ideally the device is not yet opened.
 */
QsrMailBase64Encoder::QsrMailBase64Encoder(QIODevice *device, QObject *parent) :
    QsrMailAbstractEncoder(*new QsrMailBase64EncoderPrivate(this), device, parent)
{
    Q_D(QsrMailBase64Encoder);
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
QsrMailBase64Encoder::~QsrMailBase64Encoder()
{
}

/*!
 * Return the number of bytes that are available for reading.
 */
qint64 QsrMailBase64Encoder::bytesAvailable() const
{
    Q_D(const QsrMailBase64Encoder);

    /* This is not exactly 'right' but good enough; actually there could be
     * more bytes available, but this is the safe assumption. The user will
     * check bytesAvailable() more than once anyway and everything else would
     * be guessing.
     */

    return d->device->bytesAvailable()
            + d->qSize
            + QIODevice::bytesAvailable();
}

/*!
 * Returns true if this device is sequential. This device can only be
 * sequential and thus always returns *true*.
 */
bool QsrMailBase64Encoder::isSequential() const
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
bool QsrMailBase64Encoder::open(OpenMode mode)
{
    Q_D(QsrMailBase64Encoder);

    /* avoid dupe calls to prevent signal mess */
    if (isOpen()) {
        setErrorString(tr("device already opened"));
        return false;
    }

    /* accept only read, no write */
    if (mode != QIODevice::ReadOnly) {
        setErrorString(tr("requested OpenMode not supported"));
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

    /* make sure the textmode is disabled on the underlying
     * device since we (and only we) are going to translate
     * to avoid data corruption
     */
    d->device->setTextModeEnabled(false);

    /* reset internal states */
    d->lineChars = 0;
    d->qBuffer = 0;
    d->qSize = 0;

    /* call parent implementation (never fails) */
    return QIODevice::open(mode);
}

/*!
 * Set the current position in the file. Since this implementation only
 * supports sequential access this function has no effect and always
 * returns *false*.
 */
bool QsrMailBase64Encoder::seek(qint64 pos)
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
void QsrMailBase64Encoder::setLineWidth(int width)
{
    Q_D(QsrMailBase64Encoder);
    d->lineWidth = width;
}

/*!
 * Returns the current line width in characters, excluding the CRLF at
 * the end of the line.
 */
int QsrMailBase64Encoder::lineWidth() const
{
    Q_D(const QsrMailBase64Encoder);
    return d->lineWidth;
}

/*!
 * Reimplemented for subclassing QIODevice.
 */
qint64 QsrMailBase64Encoder::readData(char *data, qint64 maxlen)
{
    Q_D(QsrMailBase64Encoder);
    return d->readDataImpl(data, maxlen);
}

/*!
 * Reimplemented for subclassing QIODevice.
 */
qint64 QsrMailBase64Encoder::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)

    return -1;
}

QT_END_NAMESPACE

#include "moc_qsrmailbase64encoder.cpp"
