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
 * \class QsrMailAbstractEncoder qsrmailabstractencoder.h <QsrMailAbstractEncoder>
 * \brief This class is the interface abstraction class for encoder devices.
 *
 * This class cannot be instanciated. Use one of the implemented encoders
 * instead.
 *
 * \sa
 * QsrMailBase64Encoder, QsrMailQpEncoder
 */

/*!
 * \internal
 *
 * \class QsrMailAbstractEncoderPrivate qsrmailabstractencoder_p.h
 * \brief The private data class of the QsrMailAbstractEncoder class.
 */

/*!
 * \internal
 *
 * \fn QsrMailAbstractEncoderPrivate::deviceAtEnd()
 * Returns true if the underlying device is at end of input. For random
 * access devices this implementation uses atEnd(), for sequential devices
 * end of input is considered to be reached when no more bytes are available
 * for read and the device emitted readChannelFinished.
 */

#include "qsrmailabstractencoder.h"
#include "qsrmailabstractencoder_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \internal
 *
 * Construct a data class for QsrMailAbstractEncoder from *qq*.
 * \param qq
 */
QsrMailAbstractEncoderPrivate::QsrMailAbstractEncoderPrivate(
        QsrMailAbstractEncoder *qq) :
    q_ptr(qq),
    device(0),
    gotReadChannelFinished(false)
{
}

/*!
 * \internal
 *
 * Destroy the instance.
 */
QsrMailAbstractEncoderPrivate::~QsrMailAbstractEncoderPrivate()
{
}

/*!
 * \internal
 *
 * Capture the readChannelFinished() event into a boolean. When the
 * event is recognized the member *gotReadChannelFinished* is set to true
 * which is required for EOF detection in sequential devices. The problem
 * with this approach is that you might miss the signal if the underlying
 * device emits the signal before the encoder is connected.
 */
void QsrMailAbstractEncoderPrivate::_q_readChannelFinished()
{
    /* remember that we got readChannelFinished; used for atEnd
     * detection of sequential devices
     */
    gotReadChannelFinished = true;
}

/*!
 * \internal
 *
 * The implementation must flush all read buffers. If there is then new data
 * is available a readyRead() signal must be emitted. The default is to do
 * nothing.
 */
void QsrMailAbstractEncoderPrivate::_q_flushBuffers()
{
    /* intentionally left blank */
}

/* -------------------------------------------------------------------------- */

/*!
 * \internal
 *
 * Create a new instance from an existing data object *dd*, the pointer to
 * the underlying *device* and make it a child to *parent*.
 */
QsrMailAbstractEncoder::QsrMailAbstractEncoder(
        QsrMailAbstractEncoderPrivate &dd, QIODevice *device, QObject *parent) :
    QIODevice(parent),
    d_ptr(&dd)
{
    Q_D(QsrMailAbstractEncoder);
    d->device = device;

    if (device->isSequential()) {
        connect(device, SIGNAL(readChannelFinished()),
                this, SLOT(_q_readChannelFinished()));
    }
}

/*!
 * Destroy the instance.
 */
QsrMailAbstractEncoder::~QsrMailAbstractEncoder()
{
}

/*!
 * Returns the underlying QIODevice.
 */
QIODevice *QsrMailAbstractEncoder::device() const
{
    Q_D(const QsrMailAbstractEncoder);
    return d->device;
}

/*!
 * Flushes the encoder buffers. The default implementation does nothing and
 * returns false.
 */
void QsrMailAbstractEncoder::flush()
{
    QMetaObject::invokeMethod(this, "_q_flushBuffers",
                              Qt::QueuedConnection);
}

QT_END_NAMESPACE

#include "moc_qsrmailabstractencoder.cpp"
