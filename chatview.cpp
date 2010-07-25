#include "chatview.h"
#include <QDebug>
#include <QWebFrame>
#include <QWebElement>

#include <KDebug>
#include <KEmoticonsTheme>
#include <KGlobal>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>


ChatView::ChatView(QWidget *parent) :
        QWebView(parent)
{
    //determine the chat window style to use (from the Kopete config file).
    //FIXME use our own config file. I think we probably want everything from the appearance config group in ours, so it's a simple change.

    KConfig config(KGlobal::dirs()->findResource("config","kopeterc"));
    KConfigGroup appearanceConfig = config.group("Appearance");

    qDebug() << QString("Loading ") << appearanceConfig.readEntry("styleName");

    m_chatStyle = new ChatWindowStyle(appearanceConfig.readEntry("styleName")); //FIXME this gets leaked !!! //FIXME hardcoded style
    if(!m_chatStyle->isValid())
    {
        KMessageBox::error(this,"Failed to load a valid Kopete theme. Note this current version reads chat window settings from your Kopete config file.");
    }
}

void ChatView::initialise(const TelepathyChatInfo &chatInfo)
{
    //Stolen from Kopete code..took out some stuff we will need in future (variants and custom Kopete style)

    //Remove silly variant fudge in here at the moment.

    QString headerHtml = replaceHeaderKeywords(m_chatStyle->getHeaderHtml(),chatInfo);


    QString xhtmlBase;

    xhtmlBase = QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                        "\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                        "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                        "<head>\n"
                        "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\n\" />\n"
                        "<base href=\"file://%1\">\n"
                        "<style id=\"baseStyle\" type=\"text/css\" media=\"screen,print\">\n"
                        "	@import url(\"main.css\");\n"
                        "   @import url(\"Yellow on Blue Alternating.css\");\n"
                        "	*{ word-wrap:break-word; }\n"
                        "</style>\n"
                        "</head>\n"
                        "<body>\n"
                        "%2\n"
                        "<div id=\"Chat\">\n</div>\n"
                        "%3\n"
                        "</body>"
                        "</html>"
                       ).arg(m_chatStyle->getStyleBaseHref()).arg(headerHtml).arg(m_chatStyle->getFooterHtml());

    setHtml(xhtmlBase);

}



void ChatView::addMessage(TelepathyChatMessageInfo & message)
{
    QWebElement chat = page()->mainFrame()->findFirstElement("#Chat");
    if (chat.isNull())
    {
        kDebug() << "Cannot find #Chat element.";
        return;
    }

    //Kopete added messages (manipulating the DOM) in a completely different way to how Adium does (via some JS)
    //Gone with the Kopete way, but the Adium way is probably worth at least considering. (the latter allows for sexy theme animations)
    QString styleHtml;

    if(message.messageDirection() == QString("rtl")) //such a hack.. put some sort of enum in the chatmessageinfoclass for type
    {
        styleHtml= m_chatStyle->getIncomingHtml();
    }
    else
    {
        styleHtml = m_chatStyle->getOutgoingHtml();
    }

    QString messageHtml = m_emoticons.theme().parseEmoticons(message.message());
    styleHtml.replace("%message%", messageHtml);
    styleHtml.replace("%messageDirection%",message.messageDirection());
    styleHtml.replace("%sender%", message.senderDisplayName()); // FIXME sender is complex: not always this
    styleHtml.replace("%time%", message.time().toString());
    styleHtml.replace("%userIconPath%", "Outgoing/buddy_icon.png");// this fallback should be done in the messageinfo

    qDebug() << styleHtml;
    chat.appendInside(styleHtml);
}


/* FIXME? maybe this method should be in the telepathychatinfo class, then not have the getters in that class.? */
QString ChatView::replaceHeaderKeywords(QString htmlTemplate, const TelepathyChatInfo & info)
{
    htmlTemplate.replace("%chatName%", info.chatName());
    htmlTemplate.replace("%sourceName%", info.sourceName());
    htmlTemplate.replace("%destinationName%", info.destinationName());
    htmlTemplate.replace("%destinationDisplayName%", info.destinationDisplayName());
    htmlTemplate.replace("%incomingIconPath%", info.incomingIconPath().toString());
    htmlTemplate.replace("%outgoingIconPath%", info.outgoingIconPath().toString());
    htmlTemplate.replace("%timeOpened%", info.timeOpened().toString()); //FIXME use KLocale to get format.
    //FIXME time fields - remember to do both, steal the complicated one from Kopete code.


    return htmlTemplate;
}
