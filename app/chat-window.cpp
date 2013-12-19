/*
    Copyright (C) 2010  David Edmundson   <kde@davidedmundson.co.uk>
    Copyright (C) 2011  Dominik Schmidt   <dev@dominik-schmidt.de>
    Copyright (C) 2011  Francesco Nwokeka <francesco.nwokeka@gmail.com>
    Copyright (C) 2013  Daniel Cohen      <analoguecolour@gmail.com>

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
#include <kparts/mainwindow.h>

ChatWindow::ChatWindow() : KXmlGuiWindow(0)
{
    setXMLFile(QLatin1String("chat-window.rc"));
    currTab;
}

void ChatWindow::setupWindow()
{
   partTabWidget->setDocumentMode(true);
   setCentralWidget(partTabWidget);
   connect(partTabWidget, SIGNAL(tabCloseRequested(int)), partTabWidget, SLOT(removeTab(int)));
   show();
   connect(partTabWidget, SIGNAL(currentChanged(int)), SLOT(onActiveTabChanged()));
   partTabWidget->setTabsClosable(true);
}

void ChatWindow::addTab(const QVariantList args, const QString ChannelAlias)
{
    KService::Ptr service = KService::serviceByDesktopPath(QString::fromLatin1("KTpTextChatPart.desktop"));
    KTpTextChatPart* part = static_cast<KTpTextChatPart*>(service->createInstance<KParts::Part>(0,  args));
    Q_ASSERT(part);
    partTabWidget->addTab(part->widget(), ChannelAlias);
    setupActions(part);
    partTabWidget->setCurrentIndex((partTabWidget->count()-1));
}

void ChatWindow::setupActions(KTpTextChatPart* part)
{
    ChatTabWidget* widget = static_cast<ChatTabWidget*>(part->widget());

    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KStandardAction::close(this, SLOT(closeCurrentTab()), actionCollection());
    KStandardAction::preferences(this, SLOT(showSettingsDialog()), actionCollection());
    KStandardAction::configureNotifications(this, SLOT(showNotificationsDialog()), actionCollection());
    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup group = config.group("Appearance");
    m_zoomFactor = group.readEntry("zoomFactor", (qreal) 1.0);
    widget->setZoomFactor(m_zoomFactor);
    KStandardAction::zoomIn(this, SLOT(onZoomIn()), actionCollection());
    KStandardAction::zoomOut(this, SLOT(onZoomOut()), actionCollection());
    connect(widget, SIGNAL(zoomFactorChanged(qreal)), this, SLOT(onZoomFactorChanged(qreal)));
    setupGUI(Default, QLatin1String("chat-window.rc"));
}

void ChatWindow::closeCurrentTab()
{
    partTabWidget->removeTab(partTabWidget->currentIndex());

}

void ChatWindow::onActiveTabChanged()
{
    if (partTabWidget->count() == 0){
        this->close();
        return;
    }
    ChatTabWidget* prevWidget = static_cast<ChatTabWidget*>(partTabWidget->widget(currTab));
    if (prevWidget != 0 && childClients().contains(prevWidget)) {
        removeChildClient(prevWidget);
    }
    ChatTabWidget* widget = static_cast<ChatTabWidget*>(partTabWidget->currentWidget());
    insertChildClient(widget);
    currTab = partTabWidget->currentIndex();
}

void ChatWindow::onZoomIn()
{
    onZoomFactorChanged(m_zoomFactor + 0.1);
}

void ChatWindow::onZoomOut()
{
    onZoomFactorChanged(m_zoomFactor - 0.1);
}

void ChatWindow::onZoomFactorChanged(qreal zoom)
{
    m_zoomFactor = zoom;

    for (int i = 0; i < partTabWidget->count(); i++) {
        ChatTabWidget* widget = static_cast<ChatTabWidget*>(partTabWidget->widget(i));
        if (!widget) {
            continue;
        }
        widget->setZoomFactor(zoom);
    }

    KConfig config(QLatin1String("ktelepathyrc"));
    KConfigGroup group = config.group("Appearance");
    group.writeEntry("zoomFactor", m_zoomFactor);
}

#include "chat-window.moc"
