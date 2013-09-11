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

#include <KDE/KGlobalSettings>
#include <KDE/KIconLoader>


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
    if (index.data(DatesModel::TypeRole).toUInt() == DatesModel::GroupRow) {
        return QSize(0, qMax(22, KGlobalSettings::smallestReadableFont().pixelSize()) + 2 + 1);
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

void DatesViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(DatesModel::TypeRole).toUInt() == DatesModel::GroupRow) {
        paintGroup(painter, option, index);
    } else {
        paintItem(painter, option, index);
    }
}

void DatesViewDelegate::paintGroup(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect groupRect = optV4.rect;

    //paint the background
    const QBrush bgBrush(option.palette.color(QPalette::Active, QPalette::Button).lighter(105));
    painter->fillRect(groupRect, bgBrush);

    //paint very subtle line at the bottom
    QPen thinLinePen;
    thinLinePen.setWidth(0);
    thinLinePen.setColor(option.palette.color(QPalette::Active, QPalette::Button));
    painter->setPen(thinLinePen);
    //to get nice sharp 1px line we need to turn AA off, otherwise it will be all blurry
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawLine(groupRect.bottomLeft(), groupRect.bottomRight());
    painter->setRenderHint(QPainter::Antialiasing, true);

    //remove spacing from the sides and one point to the bottom for the 1px line
    groupRect.adjust(0, 0, 0, -1);

    //get the proper rect for the expand sign
    int iconSize = IconSize(KIconLoader::Toolbar);

    QStyleOption expandSignOption = option;
    expandSignOption.rect = groupRect;
    expandSignOption.rect.setSize(QSize(iconSize, iconSize));
    expandSignOption.rect.moveLeft(groupRect.left());
    expandSignOption.rect.moveTop(groupRect.top() + groupRect.height()/2 - expandSignOption.rect.height()/2);

    //paint the expand sign
    if (option.state & QStyle::State_Open) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &expandSignOption, painter);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &expandSignOption, painter);
    }

    const QFont groupFont = KGlobalSettings::smallestReadableFont();
    //paint the header string
    const QRect groupLabelRect = groupRect.adjusted(expandSignOption.rect.width() + 2 * 2, 0, -2, 0);
    const QString groupHeaderString =  index.data(Qt::DisplayRole).toString();
    const QFontMetrics groupFontMetrics(groupFont);

    painter->setFont(groupFont);

    painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    painter->drawText(groupLabelRect, Qt::AlignVCenter | Qt::AlignLeft,
                      groupFontMetrics.elidedText(groupHeaderString, Qt::ElideRight,
                                                  groupLabelRect.width() - 2));

    painter->restore();
}

void DatesViewDelegate::paintItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    const int itemWidth = option.rect.width();

    int iconSize = IconSize(KIconLoader::KIconLoader::Toolbar);

    QRect itemRect = optV4.rect;
    if (index.data(DatesModel::TypeRole).toUInt() == DatesModel::ConversationRow) {
        itemRect.setX(itemRect.x() + 20);
        itemRect.setWidth(itemRect.width() - 20);
    } else {
        itemRect.setX(itemRect.x());
        itemRect.setWidth(itemRect.width());
    }

    QStyleOption expandSignOption = option;
    expandSignOption.rect = itemRect;
    expandSignOption.rect.setSize(QSize(iconSize, iconSize));
    expandSignOption.rect.moveLeft(itemRect.left());
    expandSignOption.rect.moveTop(itemRect.top() + itemRect.height()/2 - expandSignOption.rect.height()/2);

    if (index.model()->rowCount(index) > 0) {
        if (option.state & QStyle::State_Open) {
            style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &expandSignOption, painter);
        } else {
            style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &expandSignOption, painter);
        }
    }

    const QString date = index.data(Qt::DisplayRole).toString();
    QRect dateRect = itemRect;
    dateRect.setX(dateRect.x() + 20);
    dateRect.setY(dateRect.y() + (dateRect.height() / 2 - option.fontMetrics.height() / 2));
    dateRect.setWidth(qMin(option.fontMetrics.width(date) + 8, static_cast<int>(itemWidth * 2.0 / 3.0)));

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }
    painter->drawText(dateRect, option.fontMetrics.elidedText(date, Qt::ElideRight, dateRect.width()));

    const QFont hintFont = KGlobalSettings::smallestReadableFont();
    const QFontMetrics hintFontMetrics(hintFont);
    const QString hint = index.data(DatesModel::HintRole).toString();
    const int hintWidth = hintFontMetrics.width(hint);

    QRect hintRect = itemRect;
    hintRect.setX(qMax(itemWidth - 8 - hintWidth, dateRect.x() + dateRect.width()));
    hintRect.setY(hintRect.y() + (hintRect.height() - hintFontMetrics.height()));
    hintRect.setWidth(itemWidth - hintRect.x());

    painter->setFont(hintFont);
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::QPalette::Disabled, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
    }
    painter->drawText(hintRect, hintFontMetrics.elidedText(hint, Qt::ElideLeft, hintRect.width()));

    painter->restore();
}



#include "dates-view-delegate.moc"
