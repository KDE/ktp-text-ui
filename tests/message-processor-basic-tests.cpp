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

Tp::Message normalMessage(const char* msg) {
    return Tp::Message(Tp::ChannelTextMessageTypeNormal, QLatin1String(msg));
}

void MessageProcessorBasicTests::testEmoticons()
{
    QLatin1String smiley(":)");
    QLatin1String expected("<img align=\"center\" title=\":)\" alt=\":)\" src=\"/usr/share/emoticons/kde4/smile.png\" width=\"22\" height=\"22\" />\n");

    QCOMPARE(s.processOutGoingMessage(Tp::Message(Tp::ChannelTextMessageTypeNormal, smiley)).finalizedMessage(), expected);
}

void MessageProcessorBasicTests::testEscaping()
{
    Tp::Message msg = normalMessage("<script type=\"text/javascript>\nalert(\"ha!\");\n</script>");
    QString expected = QLatin1String("&lt;script type=&quot;text/javascript&gt;<br/>alert(&quot;ha!&quot;);<br/>&lt;/script&gt;\n");
    QCOMPARE(s.processOutGoingMessage(msg).finalizedMessage(), expected);
}

QTEST_MAIN(MessageProcessorBasicTests);

#include "moc_message-processor-basic-tests.cpp"