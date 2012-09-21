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
#include <QTextDocument>


#define compare(input, expected) \
{ \
    QString processed = s.getProcessedMessage(input); \
    QString href = QLatin1String(expected); \
    QCOMPARE(processed, href); \
}

#define compareWithPlainText(input, expected) \
{ \
    QString processed = s.getProcessedMessage(input); \
    QString href = QLatin1String(expected); \
    QCOMPARE(processed, href); \
    QTextDocument inputdoc, processeddoc; \
    inputdoc.setHtml(QLatin1String(input)); \
    processeddoc.setHtml(href); \
    QCOMPARE(inputdoc.toPlainText(), processeddoc.toPlainText()); \
}



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
    Message processed = this->s.processOutGoingMessage(
        Tp::Message(
            Tp::ChannelTextMessageTypeNormal,
            QLatin1String("You should consider using http://duckduckgo.com/ instead of www.google.com.au")
        )
    );

    QVariantList urls = processed.property("Urls").toList();
    QCOMPARE(urls.length(), 2);

    QCOMPARE(qvariant_cast<KUrl>(urls.at(0)), KUrl("http://duckduckgo.com/"));
    QCOMPARE(qvariant_cast<KUrl>(urls.at(1)), KUrl("http://www.google.com.au"));
}

void MessageProcessorBasicTests::testImageEmbedGIF()
{
    const char* message = "http://kde.org/images/teaser/jointhegame.gif";
    const char* imgTag =
    "<a href='http://kde.org/images/teaser/jointhegame.gif'>"
        "http://kde.org/images/teaser/jointhegame.gif"
    "</a>\n"
    "<br/><a href='http://kde.org/images/teaser/jointhegame.gif'>"
        "<img src='http://kde.org/images/teaser/jointhegame.gif'"
            " style='max-width:100%;margin-top:3px'"
            " alt='Click to view in browser' />"
    "</a>";

    compare(message, imgTag);
}

void MessageProcessorBasicTests::testXSS()
{
    compare("<script>alert('ha!');</script>", "&lt;script&gt;alert('ha!');&lt;/script&gt;");
}

// void MessageProcessorBasicTests::testSearchExpansion()
// {
    //let's assume the user hasn't messed with their web shortcuts
//     compare("gg:kde", "http://www.google.com/search?q=kde&ie=UTF-8&oe=UTF-8");
// }

void MessageProcessorBasicTests::testUsingAColon()
{
    compare("It should still leave normal stuff with:acolon alone", "It should still leave normal stuff with:acolon alone");
}

// void MessageProcessorBasicTests::testSearchExpansionWithPadding()
// {
//     compare("  gg:kde\n", "http://www.google.com/search?q=kde&ie=UTF-8&oe=UTF-8");
// }

void MessageProcessorBasicTests::testBold()
{
    compareWithPlainText("*b*", "<b>*b*</b>");
    compareWithPlainText("*bold*", "<b>*bold*</b>");
    compareWithPlainText("*this* should *be in bold*", "<b>*this*</b> should <b>*be in bold*</b>");
}

void MessageProcessorBasicTests::testItalics()
{
    compareWithPlainText("/i/", "<i>/i/</i>");
    compareWithPlainText(" /this/ should /be in italics/", "<i>/this/</i> should <i>/be in italics/</i>");
    compareWithPlainText("/all / this /should/ be in italics/", "<i>/all / this /should/</i> be in italics/");
    compareWithPlainText("/f f/ fd /f f/", "<i>/f f/</i> fd <i>/f f/</i>");
    compareWithPlainText("/f /f/", "<i>/f /f/</i>");
    compareWithPlainText("/dsd / / ss/", "<i>/dsd / / ss/</i>");
}

void MessageProcessorBasicTests::testStrikethrough()
{
    compareWithPlainText("-s-", "<s>-s-</s>");
    compareWithPlainText("-strike through-", "<s>-strike through-</s>");
}

void MessageProcessorBasicTests::testUnderline()
{
    compareWithPlainText("_u_", "<u>_u_</u>");
    compareWithPlainText("_underlined_", "<u>_underlined_</u>");
    compareWithPlainText("_this_ should _be underlined_", "<u>_this_</u> should <u>_be underlined_</u>");
}

void MessageProcessorBasicTests::testBoldItalics()
{
    compareWithPlainText("/*this is italics bold*/", "<i>/<b>*this is italics bold*</b>/</i>");
    compareWithPlainText("*/this is bold italics/*", "<b>*<i>/this is bold italics/</i>*</b>");

    compareWithPlainText("this is /*italics bold*/ , this is */bold italics/*", "this is <i>/<b>*italics bold*</b>/</i> , this is <b>*<i>/bold italics/</i>*</b>");

//    compareWithPlainText("/*this is wrong italics bold/*", "/*this is wrong italics bold/*");
//    compareWithPlainText("this has a /punctuation mark/, after the slash", "this has a <i>/punctuation mark/</i>, after the slash");
}

void MessageProcessorBasicTests::testRandomFormatting()
{
    compareWithPlainText("/this *should* work/ but /let's be *sure*/", "<i>/this <b>*should*</b> work/</i> but <i>/let's be <b>*sure*</b>/</i>");
    compareWithPlainText("/this -should- work/ and /this -should- work as well/", "<i>/this <s>-should-</s> work/</i> and <i>/this <s>-should-</s> work as well/</i>");
}


QTEST_MAIN(MessageProcessorBasicTests);

#include "moc_message-processor-basic-tests.cpp"
