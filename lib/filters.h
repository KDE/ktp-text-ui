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

#include "message-processor.h"

#include <QObject>

#include <KEmoticons>

class UrlFilter : public AbstractMessageFilter
{
public:
    explicit UrlFilter(QObject* parent = 0);
    void filterMessage(Message& message);
};

class ImageFilter : public AbstractMessageFilter
{
public:
    explicit ImageFilter(QObject* parent = 0);
    void filterMessage(Message& message);
};

class EmoticonFilter : public AbstractMessageFilter
{
public:
    explicit EmoticonFilter(QObject* parent = 0);
    void filterMessage(Message& message);
private:
    KEmoticons m_emoticons;
};