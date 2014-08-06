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

#ifndef QSRMAILRENDERER_P_H
#define QSRMAILRENDERER_P_H

#include <QObject>
#include <QQueue>
#include <QStack>
#include <QSet>
#include <QMimeDatabase>

#include "qsrmailmessage.h"
#include "qsrmailmimemultipart.h"
#include "qsrmailabstractpart_p.h"
#include "qsrmailabstractencoder.h"

QT_BEGIN_NAMESPACE

class QsrMailRenderer : public QObject
{
    Q_OBJECT

public:
    explicit QsrMailRenderer(const QsrMailMessage &message,
                             QObject *parent = 0);
    ~QsrMailRenderer();

    void setBufferSize(int size);
    int bufferSize() const;
    const char *dataPointer() const;
    int bytesAvailable() const;
    void advanceDataPointer(int bytes);
    bool atEnd() const;
    bool isRunning() const;
    QString lastError() const;

public Q_SLOTS:
    void renderMessage();
    void abort();

Q_SIGNALS:
    void readyRead();
    void readChannelFinished();
    void progressUpdate(int processed, int total);
    void error();

private:
    void setupPart(const QsrMailAbstractPartPrivate *p);
    int totalBuffers(const QsrMailAbstractPartPrivate *p);
    void enqueue(const QByteArray &chunk);
    void enqueue(QIODevice *device, bool autoDelete);
    void detachDevice();
    bool deviceAtEnd() const;

private Q_SLOTS:
    void readFromDevice();
    void endOfDevice();
    void processStates();

private:
    enum State {
        IdleState,
        SimpleBodyState,
        MimeBoundaryState,
        MimePartState,
        MimePartBodyState,
        FinishedState
    };

    struct StackFrame
    {        
        StackFrame() :
            dataP(0)
        {}

        StackFrame(const QsrMailAbstractPartPrivate *p) :
            dataP(p),
            it(dataP->parts.constBegin())
        {}

        QByteArray boundary() const;

        inline void next()
        { ++it; }

        inline bool atEnd() const
        { return dataP == 0 || it == dataP->parts.constEnd(); }

        inline const QsrMailAbstractPart &part()
        { return *it; }

        inline const QsrMailAbstractPartPrivate *partP()
        { return (*it).d.constData(); }

        const QsrMailAbstractPartPrivate *dataP;
        QList<QsrMailAbstractPart>::ConstIterator it;
    };

    /* instance data */
    State mState;
    QsrMailMimeMultipart mWrapper;
    QString mLastError;
    QIODevice *mDevice;
    QSet<QIODevice *> mDeviceFinished;
    bool mAutoDelete;
    QStack<StackFrame> mParents;
    const QsrMailAbstractPartPrivate *mPartP;
    QsrMailMimePart::Encoder mPartEncoder;

    /* ringbuffer related data */
    QByteArray mBuffer;
    const char *mReadPointer;
    int mReadPos;
    char *mWritePointer;
    int mWritePos;

    /* member data */
    const QsrMailMessagePrivate *mMessageP;
    int mTotalChunks;
    int mProcessedChunks;
};

QT_END_NAMESPACE

#endif // QSRMAILRENDERER_P_H
