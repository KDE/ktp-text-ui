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

#ifndef EMOTICONTEXTEDITSELECTOR_H
#define EMOTICONTEXTEDITSELECTOR_H

#include <QWidget>
#include <QListWidgetItem>

class ChatWindow;

class EmoticonTextEditItem : public QListWidgetItem
{
public:
  explicit EmoticonTextEditItem(const QString &emoticonText, const QString &pixmapPath, QListWidget *parent);

  QString text() const;
  QString pixmapPath() const;

private:
  QString mText;
  QString mPixmapPath;
};

class EmoticonTextEditSelector : public QWidget
{
  Q_OBJECT
public:
  explicit EmoticonTextEditSelector( ChatWindow *chatWindow, QWidget * parent = Q_NULLPTR );
  ~EmoticonTextEditSelector();

public Q_SLOTS:
  void slotCreateEmoticonList();

private Q_SLOTS:
  void slotMouseOverItem(QListWidgetItem*);
  void slotEmoticonClicked(QListWidgetItem*);

Q_SIGNALS:
  void itemSelected ( const QString& );

private:
  class EmoticonTextEditSelectorPrivate;
  EmoticonTextEditSelectorPrivate *d;
};

#endif /* EMOTICONTEXTEDITSELECTOR_H */
