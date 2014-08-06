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

#ifndef QSRMAILQPENCODER_H
#define QSRMAILQPENCODER_H

#include "qsrmailglobal.h"
#include "qsrmailabstractencoder.h"

QT_BEGIN_NAMESPACE

class QsrMailQpEncoderPrivate;
class QSRMAILSHARED_EXPORT QsrMailQpEncoder : public QsrMailAbstractEncoder
{
    Q_OBJECT

public:
    explicit QsrMailQpEncoder(QIODevice *device, QObject *parent = 0);
    ~QsrMailQpEncoder();

    qint64 bytesAvailable() const;
    bool isSequential() const;
    bool open(OpenMode mode);
    bool seek(qint64 pos);
    void setLineWidth(int value);
    int lineWidth() const;

private:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    Q_DECLARE_PRIVATE(QsrMailQpEncoder)
};

QT_END_NAMESPACE

#endif // QSRMAILQPENCODER_H
