/*
 *    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "message-processor-basic-tests.h"
#include <KUrl>

Tp::Message normalMessage(const char* msg) {
    return Tp::Message(Tp::ChannelTextMessageTypeNormal, QLatin1String(msg));
}

void MessageProcessorBasicTests::testEmoticons()
{
    QString processed = this->s.getProcessedMessage(":)");
    QString expected = QLatin1String("<img align=\"center\" title=\":)\" alt=\":)\" src=\"/usr/share/emoticons/kde4/smile.png\" width=\"22\" height=\"22\" />");

    QCOMPARE(processed, expected);
}

void MessageProcessorBasicTests::testEscaping()
{
    QString processed = this->s.getProcessedMessage("<script type=\"text/javascript>\nalert(\"ha!\");\n</script>");
    QString expected = QLatin1String("&lt;script type=&quot;text/javascript&gt;<br/>alert(&quot;ha!&quot;);<br/>&lt;/script&gt;");

    QCOMPARE(processed, expected);
}

void MessageProcessorBasicTests::testUrlCatching()
{
    QString processed = this->s.getProcessedMessage("http://www.google.com.au/");
    QString href = QLatin1String("<a href='http://www.google.com.au/'>http://www.google.com.au/</a>");

    QCOMPARE(processed, href);
}

void MessageProcessorBasicTests::testURICatchingSMB() {
    QString processed = this->s.getProcessedMessage("smb://user@localhost/");
    QString href = QLatin1String("<a href='smb://user@localhost/'>smb://user@localhost/</a>");

    QCOMPARE(processed, href);
}

void MessageProcessorBasicTests::testWWWCatching() {
    QString processed = this->s.getProcessedMessage("www.google.com.au");
    QString href = QLatin1String("<a href='http://www.google.com.au'>www.google.com.au</a>");

    QCOMPARE(processed, href);
}

void MessageProcessorBasicTests::testUnsupportedProtocolCatching()
{
    QString processed = this->s.getProcessedMessage("fakeprotocol://fakeuser@somefakehost/");
    QString href = QLatin1String("fakeprotocol://fakeuser@somefakehost/");

    QCOMPARE(processed, href);
}

void MessageProcessorBasicTests::testMetadataGeneration()
{
    Message processed = this->s.processOutGoingMessage(
        Tp::Message(
            Tp::ChannelTextMessageTypeNormal,
            QLatin1String("http://www.google.com.au/")
        )
    );

    QVariantList urls = processed.property("Urls").toList();
    QCOMPARE(urls.length(), 1);

    QCOMPARE(qvariant_cast<KUrl>(urls.at(0)), KUrl("http://www.google.com.au/"));
}

void MessageProcessorBasicTests::testMultipleURLCatching()
{
    QFAIL("not written yet");
}

QTEST_MAIN(MessageProcessorBasicTests);

#include "moc_message-processor-basic-tests.cpp"