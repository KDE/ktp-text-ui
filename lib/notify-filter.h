/*
    Copyright (C) 2013  Lasath Fernando <kde@lasath.org>

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

#ifndef NOTIFYFILTER_H
#define NOTIFYFILTER_H

#include "chat-widget.h"

#include <KTp/abstract-message-filter.h>

class NotifyFilter : public KTp::AbstractMessageFilter
{
    Q_OBJECT
public:
    explicit NotifyFilter(ChatWidget *widget);

public Q_SLOTS:
    void sendMessageNotification(const KTp::Message &message);

private:
    ChatWidget *m_widget;
};

#endif // NOTIFYFILTER_H
