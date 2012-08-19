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

#include "pipes-delegate.h"
#include "pipes-model.h"

#include <KComboBox>
#include <KDebug>

PipesDelegate::PipesDelegate(QHash< int, QString > valueNames, QObject *parent) :
    QStyledItemDelegate(parent), m_valueNames(valueNames)
{
}

PipesDelegate::~PipesDelegate()
{
}

QString PipesDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.canConvert(QVariant::Int)) {
        return m_valueNames[value.toInt()];
    }

    return QStyledItemDelegate::displayText(value, locale);
}

QWidget *PipesDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    KComboBox *box = new KComboBox(false, parent);
    Q_FOREACH (int i, m_valueNames.keys()) {
        box->insertItem(i, m_valueNames[i]);
    }
    return box;
}

void PipesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KComboBox *box = qobject_cast<KComboBox*>(editor);
    if (box) {
        box->setCurrentItem(m_valueNames[index.data().toInt()]);
    }
}

void PipesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KComboBox *box = qobject_cast<KComboBox*>(editor);
    if (box) {
        model->setData(index, m_valueNames.key(box->currentText()));
    }
}
