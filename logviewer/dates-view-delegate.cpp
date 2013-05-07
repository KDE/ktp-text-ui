/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "dates-view-delegate.h"

#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <KGlobalSettings>

#include "dates-model.h"

DatesViewDelegate::DatesViewDelegate(QObject* parent):
    QStyledItemDelegate(parent)
{
}

DatesViewDelegate::~DatesViewDelegate()
{
}

QSize DatesViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

void DatesViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    const int itemWidth = option.rect.right();

    const QString date = index.data(Qt::DisplayRole).toString();
    QRect dateRect = optV4.rect;
    dateRect.setY(dateRect.y() + (dateRect.height() / 2 - option.fontMetrics.height() / 2));
    dateRect.setWidth(qMin((int) ceil(itemWidth * 2.0 / 3.0) - optV4.rect.x(), option.fontMetrics.width(date) + optV4.rect.x()));

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }
    painter->drawText(dateRect, option.fontMetrics.elidedText(date, Qt::ElideRight, dateRect.width()));

    QFont hintFont = KGlobalSettings::smallestReadableFont();
    QFontMetrics hintFontMetrics(hintFont);

    const QString hint = index.data(DatesModel::HintRole).toString();
    QRect hintRect = optV4.rect;
    hintRect.setX(ceil((itemWidth * 2.0 / 3.0)) + 8);
    hintRect.setY(hintRect.y() + (hintRect.height() - hintFontMetrics.height()));
    hintRect.setWidth(itemWidth - hintRect.x());

    painter->setFont(hintFont);
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::QPalette::Disabled, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
    }
    painter->drawText(hintRect, hintFontMetrics.elidedText(hint, Qt::ElideRight, hintRect.width()));

    painter->restore();
}


#include "dates-view-delegate.moc"
