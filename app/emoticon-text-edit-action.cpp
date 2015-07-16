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
#include "chat-window.h"

#include <KLocalizedString>


#include <QMenu>
#include <QWidgetAction>

class EmoticonTextEditAction::EmoticonTextEditActionPrivate
{
public:
  EmoticonTextEditActionPrivate( ChatWindow *chatWindow ) {
    emoticonMenu = new QMenu();
    selector = new EmoticonTextEditSelector( chatWindow , emoticonMenu );
    QWidgetAction *action = new QWidgetAction( emoticonMenu );
    action->setDefaultWidget( selector );
    emoticonMenu->addAction( action );
    connect( emoticonMenu, SIGNAL(aboutToShow()), selector, SLOT(slotCreateEmoticonList()) );

  }
  ~EmoticonTextEditActionPrivate() {
    delete emoticonMenu;
  }

  QMenu *emoticonMenu;
  EmoticonTextEditSelector *selector;
};

EmoticonTextEditAction::EmoticonTextEditAction( ChatWindow * chatWindow )
  : KActionMenu( i18n( "Add Smiley" ), chatWindow ), d( new EmoticonTextEditActionPrivate( chatWindow ) )
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
