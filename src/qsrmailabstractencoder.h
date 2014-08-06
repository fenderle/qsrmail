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

#ifndef QSRMAILABSTRACTENCODER_H
#define QSRMAILABSTRACTENCODER_H

#include "qsrmailglobal.h"
#include <QIODevice>

QT_BEGIN_NAMESPACE

class QsrMailAbstractEncoderPrivate;
class QSRMAILSHARED_EXPORT QsrMailAbstractEncoder : public QIODevice
{
    Q_OBJECT

public:
    ~QsrMailAbstractEncoder();

    QIODevice *device() const;
    void flush();

protected:
    explicit QsrMailAbstractEncoder(QsrMailAbstractEncoderPrivate &dd,
                                    QIODevice *device, QObject *parent);

protected:
    Q_DECLARE_PRIVATE(QsrMailAbstractEncoder)

    QScopedPointer<QsrMailAbstractEncoderPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_readChannelFinished())
    Q_PRIVATE_SLOT(d_func(), void _q_flushBuffers())
};

QT_END_NAMESPACE

#endif // QSRMAILABSTRACTENCODER_H
