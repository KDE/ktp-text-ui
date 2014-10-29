/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  based on code from kopete

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/
#include "emoticon-text-edit-action.h"
#include "emoticon-text-edit-selector.h"
#include <KLocale>

#include <KMenu>

#include <QWidgetAction>

class EmoticonTextEditAction::EmoticonTextEditActionPrivate
{
public:
  EmoticonTextEditActionPrivate() {
    emoticonMenu = new KMenu();
    selector = new EmoticonTextEditSelector( emoticonMenu );
    QWidgetAction *action = new QWidgetAction( emoticonMenu );
    action->setDefaultWidget( selector );
    emoticonMenu->addAction( action );
    connect( emoticonMenu, SIGNAL(aboutToShow()), selector, SLOT(slotCreateEmoticonList()) );

  }
  ~EmoticonTextEditActionPrivate() {
    delete emoticonMenu;
  }

  KMenu *emoticonMenu;
  EmoticonTextEditSelector *selector;
};

EmoticonTextEditAction::EmoticonTextEditAction( QObject * parent )
  : KActionMenu( i18n( "Add Smiley" ), parent ), d( new EmoticonTextEditActionPrivate() )
{
  setMenu( d->emoticonMenu );
  setIcon( QIcon::fromTheme( QStringLiteral( "face-smile" ) ) );
  setDelayed( false );
  connect( d->selector, SIGNAL(itemSelected(QString)),
           this, SIGNAL(emoticonActivated(QString)) );

}

EmoticonTextEditAction::~EmoticonTextEditAction()
{
  delete d;
}

#include "emoticon-text-edit-action.moc"
