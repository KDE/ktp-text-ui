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

#include "emoticon-text-edit-selector.h"

#include <KEmoticons>
#include <kemoticonstheme.h>

#include <QListWidget>
#include <QPixmap>
#include <QHBoxLayout>

// Use a static for this as calls to the KEmoticons constructor are expensive.
Q_GLOBAL_STATIC( KEmoticons, sEmoticons )

EmoticonTextEditItem::EmoticonTextEditItem(const QString &emoticonText, const QString &pixmapPath, QListWidget *parent)
  : QListWidgetItem( parent )
{
  mText = emoticonText;
  mPixmapPath = pixmapPath;
  QPixmap p( mPixmapPath );
  // Some of the custom icons are rather large
  // so lets limit them to a maximum size for this display panel
  //
  if ( p.width() > 32 || p.height() > 32 ) {
    p = p.scaled( QSize( 32, 32 ), Qt::KeepAspectRatio );
  }

  setIcon( p );
  setToolTip( mText );
}

QString EmoticonTextEditItem::text() const
{
  return mText;
}

QString EmoticonTextEditItem::pixmapPath() const
{
  return mPixmapPath;
}

class EmoticonTextEditSelector::EmoticonTextEditSelectorPrivate
{
public:
  EmoticonTextEditSelectorPrivate() {
  }
  QListWidget *listEmoticon;
};

EmoticonTextEditSelector::EmoticonTextEditSelector( QWidget * parent )
  :QWidget( parent ), d( new EmoticonTextEditSelectorPrivate() )
{
  QHBoxLayout *lay = new QHBoxLayout( this );
  lay->setSpacing( 0 );
  lay->setContentsMargins( 0, 0, 0, 0 );
  d->listEmoticon = new QListWidget( this );
  lay->addWidget( d->listEmoticon );
  d->listEmoticon->setViewMode( QListView::IconMode );
  d->listEmoticon->setSelectionMode( QAbstractItemView::SingleSelection );
  d->listEmoticon->setMouseTracking( true );
  d->listEmoticon->setDragEnabled( false );
  connect( d->listEmoticon, SIGNAL(itemEntered(QListWidgetItem*)),
           this, SLOT(slotMouseOverItem(QListWidgetItem*)) );
  connect( d->listEmoticon, SIGNAL(itemClicked(QListWidgetItem*)),
           this, SLOT(slotEmoticonClicked(QListWidgetItem*)) );
}

EmoticonTextEditSelector::~EmoticonTextEditSelector()
{
  delete d;
}

void EmoticonTextEditSelector::slotCreateEmoticonList()
{
  d->listEmoticon->clear();
  static QString cachedEmoticonsThemeName;
  if ( cachedEmoticonsThemeName.isEmpty() ) {
    cachedEmoticonsThemeName = KEmoticons::currentThemeName();
  }
  const QHash<QString, QStringList> list = sEmoticons->theme( cachedEmoticonsThemeName ).emoticonsMap();

  QStringList emoticonKeys = list.keys();
  qSort(emoticonKeys);

  Q_FOREACH(const QString &emoticonPath, emoticonKeys) {
    new EmoticonTextEditItem(list[emoticonPath].first(), emoticonPath, d->listEmoticon);
  }

  d->listEmoticon->setIconSize( QSize( 32, 32 ) );
}


void EmoticonTextEditSelector::slotMouseOverItem(QListWidgetItem* item)
{
  item->setSelected( true );
  if ( !d->listEmoticon->hasFocus() ) {
    d->listEmoticon->setFocus();
  }
}


void EmoticonTextEditSelector::slotEmoticonClicked(QListWidgetItem*item)
{
  if ( !item ) {
    return;
  }
  EmoticonTextEditItem *itemEmoticon = static_cast<EmoticonTextEditItem*>( item );

  Q_EMIT itemSelected ( itemEmoticon->text() );
  if ( isVisible() && parentWidget() &&
       parentWidget()->inherits( "QMenu" ) ) {
    parentWidget()->close();
  }
}

#include "emoticon-text-edit-selector.moc"
