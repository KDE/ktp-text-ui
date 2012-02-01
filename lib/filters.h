
#include <QImageReader>

#include <KProtocolInfo>
#include <KDebug>

#include "message-processor.h"

class UrlFilter : public AbstractMessageFilter
{
    virtual void filterMessage(Message &info) {
        QString message = info.mainMessagePart();

        // link detection
        QRegExp linkRegExp(QLatin1String("\\b(?:(\\w+)://|(www\\.))([^\\s]+)"));
        int index = 0;

        while ((index = linkRegExp.indexIn(message, index)) != -1) {
            QString realUrl = linkRegExp.cap(0);
            QString protocol = linkRegExp.cap(1);

            //if cap(1) is empty cap(2) was matched -> starts with www.
            const bool startsWithWWW = linkRegExp.cap(1).isEmpty();

            kDebug() << "Found URL " << realUrl << "with protocol : " << (startsWithWWW ? QLatin1String("http") : protocol);


            // if url has a supported protocol
            if (startsWithWWW || KProtocolInfo::protocols().contains(protocol, Qt::CaseInsensitive)) {

                // text not wanted in a link ( <,> )
                QRegExp unwanted(QLatin1String("(&lt;|&gt;)"));

                if (!realUrl.contains(unwanted)) {
                    // string to show to user
                    QString shownUrl = realUrl;

                    // check for newline and cut link when found
                    if (realUrl.contains(QLatin1String(("<br/>")))) {
                        int findIndex = realUrl.indexOf(QLatin1String("<br/>"));
                        realUrl.truncate(findIndex);
                        shownUrl.truncate(findIndex);
                    }

                    // check prefix
                    if (startsWithWWW) {
                        realUrl.prepend(QLatin1String("http://"));
                    }

                    // if the url is changed, show in chat what the user typed in
                    QString link = QLatin1String("<a href='") + realUrl + QLatin1String("'>") + shownUrl + QLatin1String("</a>");

                    //if the link is of a supported image type, embedd it in the chat
                    QRegExp fileExtension(QLatin1String("\\.(\\w+)$"), Qt::CaseInsensitive);
                    if(fileExtension.indexIn(realUrl) != -1) {
                        QString extension = fileExtension.cap(1);
                        bool supported = false;
                        Q_FOREACH(QByteArray format, QImageReader::supportedImageFormats()) {
                            supported |= !(QString::compare(extension, QString::fromAscii(format), Qt::CaseInsensitive));
                        }
                        kDebug() << "link is supported image type ?:" << supported;
                        if(supported) {
                            link.append(QLatin1String("<br/><img src='") + realUrl + QLatin1String("' style='width:100%;' alt='(link is of an image)'/>"));
                        }
                    }
                    message.replace(index, shownUrl.length(), link);
                    // advance position otherwise I end up parsing the same link
                    index += link.length();
                } else {
                    index += realUrl.length();
                }
            } else {
                index += linkRegExp.matchedLength();
            }
        }

        info.setMainMessagePart(message);
    }
};
