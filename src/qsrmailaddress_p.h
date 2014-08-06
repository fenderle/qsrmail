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

#ifndef QSRMAILMAILADDRESS_P_H
#define QSRMAILMAILADDRESS_P_H

#include <QSharedData>
#include <QString>

QT_BEGIN_NAMESPACE

class QsrMailAddressPrivate : public QSharedData
{
public:
    QsrMailAddressPrivate();

public:
    QString address;
    QString displayName;
};

QT_END_NAMESPACE

#endif // QSRMAILMAILADDRESS_P_H
