 /*
    Copyright (C) 2013  Daniel Cohen    <analoguecolour@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "chatTabWidget.h"

#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KDebug>

ChatTabWidget::ChatTabWidget(const Tp::TextChannelPtr& channel, const Tp::AccountPtr& account, KXmlGuiWindow* parent): ChatWidget(channel, account, parent), KXMLGUIClient()
{
  KAction* findAction = new KAction(this);
  findAction->setText(i18n("&find"));
  findAction->setShortcut(Qt::CTRL + Qt::Key_F);
  actionCollection()->addAction(QLatin1String("find"), findAction);
  setXMLFile(QLatin1String("chatTabWidget.rc"));

}

ChatTabWidget::~ChatTabWidget()
{
}

#include "chatTabWidget.moc"