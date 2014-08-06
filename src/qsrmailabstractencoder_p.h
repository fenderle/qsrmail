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

#ifndef QSRMAILABSTRACTENCODER_P_H
#define QSRMAILABSTRACTENCODER_P_H

#include "qsrmailabstractencoder.h"

QT_BEGIN_NAMESPACE

class QsrMailAbstractEncoderPrivate
{
public:
    explicit QsrMailAbstractEncoderPrivate(QsrMailAbstractEncoder *qq);
    virtual ~QsrMailAbstractEncoderPrivate();

    void _q_readChannelFinished();
    virtual void _q_flushBuffers();

    inline bool deviceAtEnd() const
    {
        if (device->isSequential()) {
            return device->bytesAvailable() == 0 && gotReadChannelFinished;
        } else {
            return device->atEnd();
        }
    }

public:
    Q_DECLARE_PUBLIC(QsrMailAbstractEncoder)

    QsrMailAbstractEncoder *q_ptr;
    QIODevice *device;
    bool gotReadChannelFinished;
};

QT_END_NAMESPACE

#endif // QSRMAILABSTRACTENCODER_P_H
