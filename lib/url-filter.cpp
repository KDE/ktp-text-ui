#include "filters.h"

#include <QImageReader>

#include <KProtocolInfo>
#include <KDebug>

virtual void UrlFilter::filterMessage(Message &info) {
        QString message = info.mainMessagePart();
        //FIXME: make "Urls" into a constant
        QStringList urls = info.property("Urls");

        // link detection
        QRegExp link(QLatin1String("\\b(?:(\\w+)://|(www\\.))([^\\s]+)"));
        int fromIndex = 0;

        while ((fromIndex = message.indexOf(link, fromIndex)) != -1) {
            QString realUrl = link.cap(0);
            QString protocol = link.cap(1);

            //if cap(1) is empty cap(2) was matched -> starts with www.
            const bool startsWithWWW = link.cap(1).isEmpty();

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

                    message.replace(fromIndex, shownUrl.length(), link);
                    // advance position otherwise I end up parsing the same link
                    fromIndex += link.length();
                } else {
                    fromIndex += realUrl.length();
                }

                urls.append(realUrl);
            } else {
                fromIndex += link.matchedLength();
            }
        }

        info.setProperty("Urls", urls);
        info.setMainMessagePart(message);
    }