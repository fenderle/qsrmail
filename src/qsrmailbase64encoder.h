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

#ifndef QSRMAILBASE64ENCODER_H
#define QSRMAILBASE64ENCODER_H

#include "qsrmailglobal.h"
#include "qsrmailabstractencoder.h"

QT_BEGIN_NAMESPACE

class QsrMailBase64EncoderPrivate;
class QSRMAILSHARED_EXPORT QsrMailBase64Encoder : public QsrMailAbstractEncoder
{
    Q_OBJECT

public:
    explicit QsrMailBase64Encoder(QIODevice *device, QObject *parent = 0);
    ~QsrMailBase64Encoder();

    qint64 bytesAvailable() const;
    bool isSequential() const;
    bool open(OpenMode mode);
    bool seek(qint64 pos);
    void setLineWidth(int width);
    int lineWidth() const;

private:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    Q_DECLARE_PRIVATE(QsrMailBase64Encoder)
};

QT_END_NAMESPACE

#endif // QSRMAILBASE64ENCODER_H
