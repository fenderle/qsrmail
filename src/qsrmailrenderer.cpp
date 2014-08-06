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
 * \class QsrMailRenderer "qsrmailrenderer_p.h"
 * \brief The message renderer is used to produce the actual data representing a
 * QsrMailMessage object.
 *
 * It's optimized for non-copy and fast access, thus lacking LOTS of checks.
 * It's mostly uses pointers and tries to allocate buffers only once.
 *
 * The heart of this object is another finite state machine (FSM) which
 * handles all the internal logic. In particular this class assembles the
 * correct wire data based on the mime data, meaning it produces all the
 * headers, the boundaries and handles transfer encoding so the result is
 * "wire ready".
 *
 * It works fully asynchronous and is loosely based on the buffer class Qt
 * uses internally to buffer HTTP uploads. Like the Qt buffer class it uses a
 * buffer queue which is filled by the state machine and read by the consumer
 * when the readyRead() event fires.
 *
 * The state machine iterates through the parts of the message and creates the
 * required QIODevices on the fly, inserts the boundaries and so on. For nested
 * MIME messages it utilizes a QStack object and the StackFrame structure.
 *
 *  Color  | Usage
 *  ------ | --------------------------------------------------------
 *  green  | unibody (SimpleBody) message
 *  blue   | (nested) multipart message
 *  grey   | pseudo state which does not explicitly exists in the FSM
 *
 * \dot
 * digraph {
 *      node [shape="rectangle" fontname="sans" fontsize=9]
 *      edge [color="blue" fontname="sans" fontsize=9]
 *
 *      "Idle" [label="Idle\n(Queues message header)"];
 *      "SimpleBody" [label="SimpleBody\n(Queues message body)"];
 *      "MimeBoundary" [label="MimeBoundary\n(Queues boundary)"];
 *      "MimePart" [label="MimePart\n(Queues part header)"];
 *      "MimePartBody" [label="MimePartBody\n(Queues part body)"];
 *      "Finished" [label="Finished\n(readChannelFinished)"];
 *      "t1" [color="grey" fontcolor="grey"
 *            label="Push current Multipart\nto stack and\nset Node as current Multipart"];
 *      "t2" [color="grey" fontcolor="grey"
 *            label="Pop Multipart from stack\nand set as current Multipart"];
 *
 *      "Idle" -> "SimpleBody" [color="green" label="Unibody"];
 *      "SimpleBody" -> "Finished" [color="green" label="Mailbody"];
 *      "Idle" -> "MimeBoundary" [label="First Node\nof Multipart"];
 *      "MimeBoundary" -> "Finished" [label="No more Nodes\nand stack empty"];
 *      "MimeBoundary" -> "t2" [label="No more Nodes\nand stack not empty"];
 *      "t2" -> "MimeBoundary" [label="Next Node"]
 *      "MimeBoundary" -> "MimePart" [label="Current Node"];
 *      "MimePart" -> "t1" [label="Node is\nMultipart"];
 *      "t1" -> "MimeBoundary" [label="First Node\nof Multipart"];
 *      "MimePart" -> "MimePartBody" [label="Node is\nPart"];
 *      "MimePartBody" -> "MimeBoundary" [label="Next Node"];
 * }
 * \enddot
 *
 * After every Queued... operation the state engine is interrupted until the
 * queued data has been consumed.
 *
 * For an example on how to use this class refer to the implementation of
 * QsrMailTransport which utilizes the class for message rendering.
 */

/*!
 * \internal
 *
 * \enum QsrMailRenderer::State
 * This enum describes the states of the internal state machine.
 *
 * \var QsrMailRenderer::IdleState
 * The FSM has not yet been started and is idle.
 *
 * \var QsrMailRenderer::SimpleBodyState
 * The message contains only a simple body (no MIME message). The currently
 * active QIODevice is some form of this simple body.
 *
 * \var QsrMailRenderer::MimeBoundaryState
 * The QIODevice will output the boundary.
 *
 * \var QsrMailRenderer::MimePartState
 * The QIODevice will output the header of the current MIME part. This is also
 * where the Content-Type autodetection takes place and where the encoder is
 * selected.
 *
 * \var QsrMailRenderer::MimePartBodyState
 * The QIODevice will output the encoded version of the part body.
 *
 * \var QsrMailRenderer::FinishedState
 * The FSM has ended and readChannelFinished() has been emitted.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::readyRead()
 *
 * Emits when new data has been queued to the buffer. Use byteAvailable() to
 * retrieve the size of the buffer. dataPointer() returns a pointer to the
 * first byte of data in the buffer. Use advanceDataPointer() to move the
 * pointer through the buffer. A typical look would be:
 * \code
 * void readyRead()
 * {
 *     while (int size=renderer->bytesAvailable()) {
 *         const char *p = renderer->dataPointer();
 *         qDebug() << "DATA: " << QByteArray::fromRawData(p, size);
 *         renderer->advanceDataPointer(size);
 *     }
 * }
 * \endcode
 *
 * Obviously this make sense if you cannot process the whole buffer in one
 * readyRead() call and the buffer has to be consumed in chunks (eg. if you
 * write yourself to an unbuffered device).
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::readChannelFinished()
 *
 * Emits when the message has been processed.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::progressUpdate(int processed, int total)
 *
 * Emits when a queued buffer has been consumed using readyRead(). The object
 * has knowledge of the total number of buffers which will be queued during
 * the complete rendering cycle and passes this value as *total*. *processed*
 * is the number of the just finished buffer. This number will steadily
 * increase until *total* is reached. In other words the data can be normalized
 * to 100% which makes this suitable for progress bars.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::error()
 *
 * Emits when the renderer encounters an unresolvable problem. The detailed
 * error message is available through lastError().
 */

#include "qsrmailrenderer_p.h"
#include "qsrmailmessage_p.h"

#include "qsrmailbase64encoder.h"
#include "qsrmailqpencoder.h"

#include <QBuffer>
#include <QMimeDatabase>
#include <QStringBuilder>

QT_BEGIN_NAMESPACE

/* size of the ringbuffer - 128k should be a good value */
#define RINGBUFFER_SIZE (128*1024)

/*!
 * \internal
 *
 * Construct a new message renderer engine, based on the supplied
 * *message* and *parent*.
 */
QsrMailRenderer::QsrMailRenderer(const QsrMailMessage &message,
                                 QObject *parent) :
    QObject(parent),
    mState(IdleState),
    mDevice(0),
    mAutoDelete(false),
    mPartP(0),
    mPartEncoder(QsrMailMimePart::AutoDetectEncoder),
    mBuffer(QByteArray(RINGBUFFER_SIZE, Qt::Uninitialized)),
    mReadPointer(mBuffer.constData()),
    mReadPos(0),
    mWritePointer(mBuffer.data()),
    mWritePos(0),
    mMessageP(message.d.constData()),
    mTotalChunks(0),
    mProcessedChunks(0)
{
    /* setup parts for rendering - involves connecting readChannelFinished() */
    setupPart(mMessageP->body.d.constData());
}

/*!
 * \internal
 *
 * Default destructor.
 */
QsrMailRenderer::~QsrMailRenderer()
{
}

/*!
 * \internal
 *
 * Sets the ringbuffer to *size*. Usually the default oh 128kByte should
 * be more than enough for buffering. This method exists so the parameter
 * can potentially be exposed to the API.
 */
void QsrMailRenderer::setBufferSize(int size)
{
    /* be careful */
    if (isRunning()) {
        qWarning("QsrMailRenderer::setBufferSize: " \
                 "cannot change ringbuffer size while rendering");
        return;
    }

    mBuffer = QByteArray(size, Qt::Uninitialized);
}

/*!
 * \internal
 *
 * Returns the max size of the buffer. This is the value set using
 * setBufferSize() and defaults to 128kByte
 */
int QsrMailRenderer::bufferSize() const
{
    return mBuffer.size();
}

/*!
 * \internal
 *
 * A direct pointer to the current data chunk. Use bytesAvailable()
 * to find the number of bytes which are safely available in the
 * buffer. *DO NOT READ BEYOND THE END OF THE BUFFER*. Also be
 * aware that the returned pointer might actually be null.
 */
const char *QsrMailRenderer::dataPointer() const
{
    return mReadPointer;
}

/*!
 * \internal
 *
 * Number of bytes available to read, starting at dataPointer().
 */
int QsrMailRenderer::bytesAvailable() const
{
    return mWritePos - mReadPos;
}

/*!
 * \internal
 *
 * Advances the dataPointer() by the given number of *bytes*. If the
 * end of the current buffer is reached the FSM is kicked off to gather
 * more data. Upon more data is queued the readyRead() signal will be
 * fired.
 */
void QsrMailRenderer::advanceDataPointer(int bytes)
{
    /* shift pointer and adjust size of chunk */
    mReadPointer += bytes;
    mReadPos += bytes;

    if (mReadPos >= mWritePos) {        
        /* handle buffer wrap */
        if (mWritePos == mBuffer.size()) {
            mReadPointer = mBuffer.constData();
            mReadPos = 0;
            mWritePointer = mBuffer.data();
            mWritePos = 0;
        }

        if (mDevice == 0) {
            /* if there's no device open we must trigger the FSM to
             * produce more content - this will emit chunk ready...
             */

            QMetaObject::invokeMethod(this, "processStates",
                                      Qt::QueuedConnection);
        } else {
            /* if there's a device we just try to read once more now
             * or wait for the next readyRead() signal
             */
            QMetaObject::invokeMethod(this, "readFromDevice",
                                      Qt::QueuedConnection);
        }
    }
}

/*!
 * \internal
 *
 * Returns true if end of the message has been reached.
 */
bool QsrMailRenderer::atEnd() const
{
    return mState == FinishedState && mDevice == 0 && mReadPos >= mWritePos;
}

/*!
 * \internal
 *
 * Returns true if the state machine is running
 */
bool QsrMailRenderer::isRunning() const
{
    return mState != IdleState && mState != FinishedState;
}

/*!
 * \internal
 *
 * Returns the renderers last error message. The returned string is null in
 * case no error occured.
 */
QString QsrMailRenderer::lastError() const
{
    return mLastError;
}

/*!
 * \internal
 *
 * Start processing the message.
 *
 * \warning
 * It is not possible to start the renderer a second time since all
 * QIODevices in the QsrMailMessage have been consumed and their end of data
 * is reached when rendering has finished. Starting a second time will NOT
 * produce the same result! You have to reset the whole QsrMailMessage which
 * results in instanciating a new renderer.
 */
void QsrMailRenderer::renderMessage()
{
    /* the FSM cannot be reused - also prevents confusing running FSM */
    if (mState != IdleState) {
        qWarning("cannot reuse QsrMailRenderer objects.");
        return;
    }

    /* trigger start */
    QMetaObject::invokeMethod(this, "processStates", Qt::QueuedConnection);
}

/*!
 * \internal
 *
 * Abort the render processing. No more signals will be emitted (not
 * even the finished signal), the state machine jumps immediately to the
 * finished state and all resources will be released (including queued
 * buffers).
 */
void QsrMailRenderer::abort()
{
    if (mDevice != 0)
        detachDevice();

    mReadPointer = mBuffer.constData();
    mReadPos = 0;

    mWritePointer = mBuffer.data();
    mWritePos = 0;

    mState = FinishedState;
}

/*!
 * \internal
 *
 * Setup a part and all of it's children. The purpose is to do everything
 * required to get message parts ready for delivery. E.g. it connects
 * the devices readChannelFinished() signal.
 *
 * This method is called at a very early stage of message processing while
 * the construction of QsrMailRenderer, which usually is performed while
 * queueing the message.
 */
void QsrMailRenderer::setupPart(const QsrMailAbstractPartPrivate *p)
{
    if (p->isMimeMultipart()) {
        foreach (const QsrMailAbstractPart &part, p->parts)
            setupPart(part.d.constData());
    } else {
        if (p->bodyDevice != 0) {
            /* setup readChannelFinished */
            connect(p->bodyDevice, SIGNAL(readChannelFinished()),
                    this, SLOT(endOfDevice()));
        }
    }
}

/*!
 * \internal
 *
 * Returns the number of buffers a message *p* will produce. This is done by
 * resursively counting the message parts and adding the fixed values for
 * boundaries, headers and so on.
 */
int QsrMailRenderer::totalBuffers(const QsrMailAbstractPartPrivate *p)
{
    if (p->isMimePart()) {
        return 1;
    } else if (p->isMimeMultipart()) {
        /* header */
        int result = 1;

        /* each part... */
        foreach (const QsrMailAbstractPart &part, p->parts) {
            /* boundary, header and parts */
            result += 2;
            result += totalBuffers(part.d.constData());
        }

        /* final boundary */
        result += 1;

        return result;
    } else {
        /* invalid? huh?? anyway return 0 */
        return 0;
    }
}

/*!
 * \internal
 *
 * Creates and queues a QIODevice with the contents of *chunk*. The device
 * will be opened and the first readFromDevice() is triggered.
 */
void QsrMailRenderer::enqueue(const QByteArray &chunk)
{
    Q_ASSERT(mDevice == 0);

    QBuffer *device = new QBuffer(this);
    device->setData(chunk);

    mDevice = device;
    mDevice->open(QIODevice::ReadOnly);
    mAutoDelete = true;

    connect(mDevice, SIGNAL(readyRead()), this, SLOT(readFromDevice()));
    QMetaObject::invokeMethod(this, "readFromDevice", Qt::QueuedConnection);
}

/*!
 * \internal
 *
 * Queues the QIODevice *device*. It is assured that the device is opened and
 * ready to read and the initial readFromDevice() is triggered.
 */
void QsrMailRenderer::enqueue(QIODevice *device, bool autoDelete)
{
    Q_ASSERT(device != 0 && mDevice == 0);

    /* make sure the device is accessible */
    if (!device->isReadable()) {
        if (!device->isOpen()) {
            if (!device->open(QIODevice::ReadOnly)) {
                mLastError = tr("cannot open attachment for reading: ")
                        % device->errorString();
                emit error();
                return;
            }
        }
    }

    /* check access again */
    if (!device->isReadable()) {
        mLastError = tr("cannot read attachment");
        emit error();
        return;
    }

    /* device is valid - remember it */
    mDevice = device;
    mAutoDelete = autoDelete;

    connect(mDevice, SIGNAL(readyRead()), this, SLOT(readFromDevice()));
    QMetaObject::invokeMethod(this, "readFromDevice", Qt::QueuedConnection);
}

/*!
 * \internal
 *
 * Cleanup the internal device pointer for external data sources.
 * This disconnects the event chain and makes sure the device is not
 * left over if it was actually allocated by the renderer.
 */
void QsrMailRenderer::detachDevice()
{
    Q_ASSERT(mDevice != 0);

    /* disconnect us from the object */
    mDevice->disconnect(this);

    /* unwrap the encoder */
    QsrMailAbstractEncoder *encoder =
            qobject_cast<QsrMailAbstractEncoder *>(mDevice);
    if (encoder != 0) {
        mDevice = encoder->device();
        encoder->close();
        encoder->deleteLater();
    }

    /* delete the device if requested */
    if (mAutoDelete) {
        mDevice->close();
        mDevice->deleteLater();
    }

    /* ... no longer processing a device */
    mDevice = 0;    
}

/*!
 * \internal
 *
 * Detect the end of an device. For the renderer it is a problem to really
 * know if a device is at it's read end or not. This is because sequential
 * and random access devices handle atEnd() very differently. While for
 * random access devices atEnd() represents the 'end of the device', for
 * sequential devices this signals only the 'end of read buffer'. So for
 * sequential devices we must track the readChannelFinished() signal, which
 * is emitted when the device reaches the end of the device - and because
 * the signal is emitted as soon as the device end is reached it does not
 * indicate that the buffers are depleted also, meaning we have to check
 * the input buffers too. Anyhow. Since we could miss the firing of the
 * signal when the device finishes *before* we are reading it, the best
 * solution I came up with was to connect the device *immediately*
 * instanciation of the renderer which conincides with message queueing.
 * The handler writes the QIODevice pointer to a QSet when it receives the
 * signal.
 *
 * What this function does is to try really hard to determine the correct
 * device atEnd() result. Essentially for encoder wrapped devices the device
 * is unwrap and depending on if it is a sequential device or not different
 * strategies take place to determine end of file.
 */
bool QsrMailRenderer::deviceAtEnd() const
{
    QsrMailAbstractEncoder *encoder =
            qobject_cast<QsrMailAbstractEncoder *>(mDevice);
    if (encoder != 0) {
        /* Check if buffers are empty - the encoder also checks the underlying
         * device so we do not have to take care of that
         */
        QIODevice *device = encoder->device();
        bool buffersEmpty = encoder->bytesAvailable() == 0;

        /* When the device is depleted, but there's still data in the
         * encoder buffer then flush the encoder. The method will emit
         * readyRead.
         */
        if (!buffersEmpty && mDeviceFinished.contains(device)) {
            encoder->flush();
            return false;
        }

        /* Determine EOF based on device type and buffer state */
        if (device->isSequential())
            return buffersEmpty && mDeviceFinished.contains(device);
        else
            return buffersEmpty && device->atEnd();
    } else {
        bool buffersEmpty = mDevice->bytesAvailable() == 0;

        /* Determine EOF based on device type and buffer state */
        if (mDevice->isSequential())
            return buffersEmpty && mDeviceFinished.contains(mDevice);
        else
            return buffersEmpty && mDevice->atEnd();
    }
}

/*!
 * \internal
 *
 * Device reader for the currently processing QIODevice. This reads from the
 * QIODevice and fills the ringbuffer. Emits progressUpdate() and readyRead()
 * signals in the process.
 */
void QsrMailRenderer::readFromDevice()
{
    /* make sure the device is valid since the device might have
     * been disposed meanwhile (eg. by abort())
     */
    if (mDevice == 0)
        return;

    /* try to fill the buffer */
    bool dataAvailable = false;
    qint64 remaining = mBuffer.size() - mWritePos;
    while (remaining > 0) {
        qint64 got = mDevice->read(mWritePointer, remaining);

        /* handle error */
        if (got < 0) {
            mLastError = tr("read error from device: ")
                    % mDevice->errorString();
            emit error();
            break;
        }

        /* no more data available? */
        if (got == 0)
            break;

        /* adjust pointers, etc. */
        remaining -= got;

        mWritePos += got;
        mWritePointer += got;

        dataAvailable = true;
    }

    /* Detach the device if all data has been read. Be aware
     * that sequential QIODevices need different handling of EOF.
     */
    if (deviceAtEnd()) {
        /* chunk is complete */
        mProcessedChunks++;
        detachDevice();

        emit progressUpdate(mProcessedChunks, mTotalChunks);

        /* in case we got no more data we trigger the FSM to jump
         * to the next state
         */
        if (!dataAvailable) {
            QMetaObject::invokeMethod(this, "processStates",
                                      Qt::QueuedConnection);
        }
    }

    /* if there was anything buffered emit the signal */
    if (dataAvailable)
        emit readyRead();
}

/*!
 * \internal
 *
 * One of the attached devices sent readChannelFinished(). Remembers the
 * QIODevice pointer in the mDeviceFinished QSet for lookup while checking
 * for atEnd() of particular devices.
 */
void QsrMailRenderer::endOfDevice()
{
    mDeviceFinished.insert(qobject_cast<QIODevice *>(QObject::sender()));
}

/*!
 * \internal
 *
 * Advance the FSM one state. When this function returns the FSM is either
 * in FinishedState or a QIODevice has been queued for processing. This is
 * also the 'mainloop' where all the magic happens.
 */
void QsrMailRenderer::processStates()
{
    switch (mState) {
    case IdleState: {
        /* we start with the body */
        mPartP = mMessageP->body.d.constData();

        /* create message headers */
        QByteArray headers;
        QsrMailHeaders messageHeaders(mMessageP->cookHeaders());

        /* react on the different types */
        switch (mPartP->partType) {
        case QsrMailAbstractPart::MimePartType:
            /* we need to wrap the part into a multipart - this is a bit ugly */
            mWrapper.d.data()->parts.append(mMessageP->body);
            mPartP = mWrapper.d.constData();

            QSRMAIL_FALLTHROUGH;

        case QsrMailAbstractPart::MimeMultipartType:
            /* the mail header itself needs not to be counted as it is part of
             * the multipart header
             */
            mProcessedChunks = 0;
            mTotalChunks = totalBuffers(mPartP);
            emit progressUpdate(mProcessedChunks, mTotalChunks);

            /* output mime version and multipart headers */
            messageHeaders.setHeader("MIME-Version", "1.0");
            headers = messageHeaders.renderHeaders()
                    % mPartP->cookHeaders().renderHeaders()
                    % "\r\n";
            enqueue(headers);

            /* put the multipart on the stack and branch to the
             * boundary processing
             */
            mParents.push(StackFrame(mPartP));

            mState = MimeBoundaryState;
            break;

        case QsrMailAbstractPart::NullType:
        case QsrMailAbstractPart::BodyPartType:
            /* everything else is handled as bodyPart */
            mProcessedChunks = 0;
            mTotalChunks = 2;
            emit progressUpdate(mProcessedChunks, mTotalChunks);

            /* output end-of-header sig. */
            headers = messageHeaders.renderHeaders() % "\r\n";
            enqueue(headers);

            mState = SimpleBodyState;
            break;
        }
        break;
    }

    case SimpleBodyState: {
        /* enqueue body or body device */
        if (mPartP->bodyDevice == 0)
            enqueue(mPartP->body);
        else
            enqueue(mPartP->bodyDevice, mPartP->autoDelete);

        mState = FinishedState;
        break;
    }

    case MimeBoundaryState: {
        /* not all parts have been processed for this parent, so output
         * a boundary. Note that the boundary() helper function takes
         * care of formatting the boundary correctly depending on the
         * parts position in the part list (that is first boundary, middle
         * boundaries and last boundary calls act different)
         */
        enqueue(mParents.top().boundary());

        if (mParents.top().atEnd()) {
            /* no more parts in this list - cleanup and switch to
             * the next part
             */
            mParents.pop();
            if (mParents.isEmpty()) {
                /* everything has been processed */
                mState = FinishedState;
            } else {
                /* output next boundary */
                mParents.top().next();
                mState = MimeBoundaryState;
            }
        } else {
            /* output part header */
            mState = MimePartState;
        }
        break;
    }

    case MimePartState: {
        mPartP = mParents.top().partP();

        if (mPartP->isMimeMultipart()) {
            /* part is multipart -> push it to the stack */
            mParents.push(StackFrame(mPartP));

            /* setup multipart headers */
            QByteArray headers(mPartP->cookHeaders().renderHeaders());
            headers += "\r\n";
            enqueue(headers);

            mState = MimeBoundaryState;
            break;
        }

        /* switch to next part */
        mParents.top().next();

        /* setup part data iterator */
        mPartEncoder = mPartP->encoder;

        /* build part headers */
        QsrMailHeaders headerList(mPartP->cookHeaders());

        /* determine content type */
        QByteArray contentType(headerList.value("Content-Type"));
        if (contentType.isEmpty()) {
            /* detect contentType using QMimeDatabase */
            QMimeDatabase mimeDb;
            QMimeType mimeType;

            if (mPartP->bodyDevice == 0)
                mimeType = mimeDb.mimeTypeForData(mPartP->body);
            else
                mimeType = mimeDb.mimeTypeForData(mPartP->bodyDevice);

            /* extract content type - just convert it to latin1. this
             * should not be a problem since mime types are usually
             * US-ASCII
             */
            contentType = mimeType.name().toLatin1();

            /* fallback to RFC default if type cannot be determined */
            if (contentType.isEmpty())
                contentType = "text/plain; charset=us-ascii";

            /* override the Content-Type header */
            headerList.setHeader("Content-Type", contentType);
        }

        /* determine encoder */

        /* autodetect selects quoted printable for text class,
         * base64 for everything else
         */
        if (mPartEncoder == QsrMailMimePart::AutoDetectEncoder) {
            if (contentType.startsWith("text/"))
                mPartEncoder = QsrMailMimePart::QuotedPrintableEncoder;
            else
                mPartEncoder = QsrMailMimePart::Base64Encoder;
        }

        /* override content transfer encoding based on selected encoder */
        if (mPartEncoder == QsrMailMimePart::Base64Encoder) {
            headerList.setHeader("Content-Transfer-Encoding",
                                 "base64");
        } else if (mPartEncoder == QsrMailMimePart::QuotedPrintableEncoder) {
            headerList.setHeader("Content-Transfer-Encoding",
                                 "quoted-printable");
        }

        /* render the headers for the part */
        QByteArray headers(headerList.renderHeaders());
        headers += "\r\n";
        enqueue(headers);

        mState = MimePartBodyState;
        break;
    }

    case MimePartBodyState: {
        /* enqueue depending on encoder */
        if (mPartEncoder == QsrMailMimePart::PassthroughEncoder) {
            /* passthrough encoder simply enqueues the data */
            if (mPartP->bodyDevice == 0)
                enqueue(mPartP->body);
            else
                enqueue(mPartP->bodyDevice, mPartP->autoDelete);
        } else {
            /* if an encoder is requested it will wrap the original data
             * or device
             */

            /* construct a base device, which is either the originally
             * supplied device or a QBuffer which wraps the QByteArray
             * holding the content data
             */
            QIODevice *base = 0;
            bool autoDelete = false;

            if (mPartP->bodyDevice == 0) {
                QBuffer *buffer = new QBuffer;
                buffer->setData(mPartP->body);
                base = buffer;
                autoDelete = true;
            } else {
                base = mPartP->bodyDevice;
                autoDelete = mPartP->autoDelete;
            }

            /* wrap the base device with the encoder - welcome to
             * the onion!
             */
            QIODevice *wrapper = base;

            switch (mPartEncoder) {
            case QsrMailMimePart::Base64Encoder:
                wrapper = new QsrMailBase64Encoder(base, this);
                wrapper->open(QIODevice::ReadOnly);
                break;
            case QsrMailMimePart::QuotedPrintableEncoder:
                wrapper = new QsrMailQpEncoder(base, this);
                wrapper->open(QIODevice::ReadOnly);
                break;

            case QsrMailMimePart::AutoDetectEncoder:
            case QsrMailMimePart::PassthroughEncoder:
                /* if this happens someone added a new encoder and forgot
                 * to add it here or something is really wrong ;-)
                 */
                qFatal("QsrMailMsgRendererPrivate::processState: " \
                       "invalid encoder (encoder=%d)", mPartEncoder);
            }

            /* queue the wrapper */
            enqueue(wrapper, autoDelete);
        }

        mState = MimeBoundaryState;
        break;
    }

    case FinishedState: {
        /* FSM has ended - the message is now complete */
        emit readChannelFinished();
        break;
    }

    }
}

/*!
 * \internal
 *
 * \class QsrMailRenderer::StackFrame
 * \brief Helper class used to handle the stack for nested multiparts.
 *
 * This class also provides a few handy helper methods to make life a little
 * easier.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::StackFrame::StackFrame();
 * Default constructor required for use in QList.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::StackFrame::StackFrame(const QsrMailAbstractPartPrivate *d)
 * Construct a new StackFrame object based on a part implementation
 * object.
 *
 * *d* is a pointer to the multiparts implementation instance. The
 * object is *not* copied and it must be ensured that the pointer
 * stays valid over the lifetime of this object.
 */

/*!
 * \internal
 *
 * Create a valid boundary depending on the boundary set in the
 * multipart data and the position of the mime part in the
 * multiparts part list. In detail this affects:
 *
 * * if the mime part is the first in the list a CRLF sequence
 *   is prepended in order to terminate the header list which
 *   preceeds the first boundary.
 * * parts between the first and the last part just output the
 *   boundary as usual
 * * if the part is the last in the list the termination sequence
 *   (-- at the end of the boundary) is added.
 */
QByteArray QsrMailRenderer::StackFrame::boundary() const
{
    QByteArray result;

    if (it != dataP->parts.constBegin()) {
        if ((it-1)->d.constData()->isMimePart())
            result = "\r\n";
    }

    result += "--";
    result += dataP->boundary;

    if (it == dataP->parts.constEnd())
        result += "--";

    result += "\r\n";

    return result;
}

/*!
 * \internal
 *
 * \fn QsrMailRenderer::StackFrame::next()
 *
 * Advance the iterator to the next part.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::StackFrame::atEnd()
 *
 * Return true if the the iterator has reached the end of
 * the multipart list.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::StackFrame::part()
 *
 * Returns a pointer to the current part addressed by the iterator.
 */

/*!
 * \internal
 *
 * \fn QsrMailRenderer::StackFrame::partP()
 *
 * Returns a pointer to the current part's private data addressed by
 * the iterator.
 */

QT_END_NAMESPACE
