/***************************************************************************
 *   Copyright (C) 2010 by David Edmundson <kde@davidedmundson.co.uk>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef CHATVIEW_H
#define CHATVIEW_H

#include <QWebView>
#include "chatwindowstyle.h"
#include "telepathychatmessageinfo.h"
#include "telepathychatinfo.h"
#include <KEmoticons>



class ChatView : public QWebView
{
    Q_OBJECT
public:
    explicit ChatView(QWidget *parent = 0);
    void initialise(const TelepathyChatInfo&);

public slots:
    void addMessage(TelepathyChatMessageInfo & message);

private:
    ChatWindowStyle* m_chatStyle;
    QString m_variantPath;
    KEmoticons m_emoticons;
    QString replaceHeaderKeywords(QString htmlTemplate, const TelepathyChatInfo&);
    //QString replaceMessageKeywords(QString htmlTemplate, const TelepathyChatMessageInfo&);


    QString lastSender;
    bool m_showHeader;

    void appendNewMessage(QString);
    void appendNextMessage(QString);

    bool m_webInspector;
};

#endif // CHATVIEW_H
