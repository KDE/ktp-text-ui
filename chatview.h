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
signals:

public slots:
    void addMessage(TelepathyChatMessageInfo & message);

private:
    ChatWindowStyle* m_chatStyle;
    KEmoticons m_emoticons;
    QString replaceHeaderKeywords(QString htmlTemplate, const TelepathyChatInfo&);
//replaceMessageKeywords(QString htmlTemplate, const TelepathyChatMessageInfo&);
};

#endif // CHATVIEW_H
