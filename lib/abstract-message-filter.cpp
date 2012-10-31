/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "abstract-message-filter.h"
#include <KSharedConfig>

AbstractMessageFilter::AbstractMessageFilter(QObject* parent)
    : QObject(parent)
{
}

AbstractMessageFilter::~AbstractMessageFilter()
{
}

void AbstractMessageFilter::filterIncomingMessage(Message &message)
{
    filterMessage(message);
}

void AbstractMessageFilter::filterOutgoingMessage(Message &message)
{
    filterMessage(message);
}

void AbstractMessageFilter::filterMessage(Message &message)
{
    Q_UNUSED(message)
}

QStringList AbstractMessageFilter::requiredScripts()
{
    return QStringList();
}

QStringList AbstractMessageFilter::requiredStylesheets()
{
    return QStringList();
}

KConfigGroup AbstractMessageFilter::config()
{
    // is there a way to make sure the derived class is a Q_OBJECT,
    // and therefore have a differenct objectName?
    return KSharedConfig::openConfig(QLatin1String("ktelepathyrc"))->group("Filters").group(objectName());
}
