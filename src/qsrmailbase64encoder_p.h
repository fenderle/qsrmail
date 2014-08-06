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

#ifndef QSRMAILBASE64ENCODER_P_H
#define QSRMAILBASE64ENCODER_P_H

#include "qsrmailabstractencoder_p.h"
#include "qsrmailbase64encoder.h"

QT_BEGIN_NAMESPACE

/* forward declaration for inline */
extern const char base64dict[];

class QsrMailBase64EncoderPrivate : public QsrMailAbstractEncoderPrivate
{
public:
    explicit QsrMailBase64EncoderPrivate(QsrMailBase64Encoder *qq);
    ~QsrMailBase64EncoderPrivate();

    void _q_flushBuffers();
    qint64 readDataImpl(char *data, qint64 maxlen);

    inline int put(char **p, char c) {
        *(*p)++ = c;

        if (lineWidth > 0 && ++lineChars >= lineWidth) {
            *(*p)++ = '\r';
            *(*p)++ = '\n';
            lineChars = 0;
            return 3;
        }

        return 1;
    }

    inline int putQ(char **p) {
        int pad = 3 - qSize;
        int result = 0;
        result += put(p, base64dict[(qBuffer & 0xfc0000) >> 18]);
        result += put(p, base64dict[(qBuffer & 0x3f000) >> 12]);
        result += put(p, pad == 2 ? '=' : base64dict[(qBuffer & 0xfc0) >> 6]);
        result += put(p, pad >= 1 ? '=' : base64dict[qBuffer & 0x3f]);

        qBuffer = 0;
        qSize = 0;

        return result;
    }

public:
    Q_DECLARE_PUBLIC(QsrMailBase64Encoder)

    int lineWidth;
    int lineChars;
    quint32 qBuffer;
    int qSize;
};

QT_END_NAMESPACE

#endif // QSRMAILBASE64ENCODER_P_H
