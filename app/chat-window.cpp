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

#include "chat-window.h"
#include "chatTabWidget.h"
#include "kxmlguiclient.h"

#include <kservice.h>
#include <KApplication>
#include <KAction>
#include <KLocale>
#include <KActionCollection>
#include <KStandardAction>
#include <KDebug>

ChatWindow::ChatWindow() : KXmlGuiWindow(0)
{
    setXMLFile(QLatin1String("chat-window.rc"));
}

ChatWindow::~ChatWindow()
{
}

void ChatWindow::setupWindow()
{
   partTabWidget->setDocumentMode(true);
   partTabWidget->setTabBarHidden(true);
   setCentralWidget(partTabWidget);
   QObject::connect(partTabWidget, SIGNAL(tabCloseRequested(int)), partTabWidget, SLOT(removeTab(int)));
   show();
}

void ChatWindow::addTab(QVariantList args, QString channelalias)
{
    KService::Ptr service = KService::serviceByDesktopPath(QString::fromLatin1("KTpTextChatPart.desktop"));
    KTpTextChatPart* part = static_cast<KTpTextChatPart*>(service->createInstance<KParts::Part>(0,  args));
    Q_ASSERT(part);
    partTabWidget->addTab(part->widget(), channelalias);
    setupActions(part);
}

void ChatWindow::setupActions(KTpTextChatPart* part)
{
    KStandardAction::close(this, SLOT(close()), actionCollection());
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KAction* closeAction = new KAction(this);
    closeAction->setText(i18n("&Close"));
    closeAction->setShortcut(Qt::CTRL + Qt::Key_W);
    actionCollection()->addAction(QLatin1String("close"), closeAction);
    ChatTabWidget* widget = static_cast<ChatTabWidget*>(part->widget());
    insertChildClient(widget);
    setupGUI(Default, QLatin1String("chat-window.rc"));
}

#include "chat-window.moc"
