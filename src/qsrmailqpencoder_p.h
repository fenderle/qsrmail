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

#ifndef QSRMAILQPENCODER_P_H
#define QSRMAILQPENCODER_P_H

#include "qsrmailabstractencoder_p.h"
#include "qsrmailqpencoder.h"

QT_BEGIN_NAMESPACE

class QsrMailQpEncoderPrivate : public QsrMailAbstractEncoderPrivate
{
public:
    explicit QsrMailQpEncoderPrivate(QsrMailQpEncoder *qq);
    ~QsrMailQpEncoderPrivate();

    qint64 readDataImpl(char *data, qint64 maxlen);

public:
    Q_DECLARE_PUBLIC(QsrMailQpEncoder)

    int lineWidth;
    int lineChars;
};

QT_END_NAMESPACE

#endif // QSRMAILQPENCODER_P_H
