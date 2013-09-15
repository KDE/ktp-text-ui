/*
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
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

#include "entity-view-delegate.h"
#include "person-entity-merge-model.h"

#include <QtGui/QPainter>
#include <QtGui/QApplication>

#include <KDE/KIconLoader>
#include <KDE/KGlobalSettings>
#include <KDE/KDebug>

#include <TelepathyQt/Account>

const qreal GROUP_ICON_OPACITY = 0.6;

EntityViewDelegate::EntityViewDelegate(QObject* parent):
    QStyledItemDelegate(parent),
    m_avatarSize(IconSize(KIconLoader::Toolbar)),
    m_spacing(2)
{
}

EntityViewDelegate::~EntityViewDelegate()
{
}

void EntityViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(PersonEntityMergeModel::ItemTypeRole).toUInt() == PersonEntityMergeModel::Group) {
        paintHeader(painter, option, index);
    } else {
        paintContact(painter, option, index);
    }
}

QSize EntityViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(PersonEntityMergeModel::ItemTypeRole).toInt() == PersonEntityMergeModel::Group) {
        return sizeHintHeader(option, index);
    } else {
        return sizeHintContact(option, index);
    }
}

void EntityViewDelegate::paintContact(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    const bool isSubcontact = index.parent().data(PersonEntityMergeModel::ItemTypeRole).toUInt() == PersonEntityMergeModel::Persona;
    const bool isEntity = index.data(PersonEntityMergeModel::ItemTypeRole).toUInt() == PersonEntityMergeModel::Entity;

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &optV4, painter);

    if (isSubcontact) {
        optV4.rect.setLeft(optV4.rect.left() + 10);
    }

    QRect iconRect = optV4.rect;
    iconRect.setSize(QSize(m_avatarSize, m_avatarSize));
    iconRect.moveTo(QPoint(iconRect.x() + m_spacing, iconRect.y() + m_spacing));

    const QVariant var = index.data(Qt::DecorationRole);
    QPixmap avatar;
    if (var.canConvert<QPixmap>()) {
        avatar = var.value<QPixmap>();
    } else if (var.canConvert<QIcon>()) {
        avatar = var.value<QIcon>().pixmap(QSize(m_avatarSize, m_avatarSize));
    }

    //some avatars might be invalid and their file not exist anymore,
    //so we need to check if the pixmap is not null
    if (avatar.isNull()) {
        avatar = SmallIcon(QLatin1String("im-user"), KIconLoader::SizeMedium);//avatar.load("/home/mck182/Downloads/dummy-avatar.png");
    }

    style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, avatar.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    const QFont nameFont = KGlobalSettings::generalFont();
    const QFontMetrics nameFontMetrics(nameFont);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    const QString nameText = index.data(Qt::DisplayRole).toString();

    painter->setFont(nameFont);

    QRect userNameRect = optV4.rect;
    userNameRect.setX(iconRect.x() + iconRect.width() + m_spacing * 2);
    userNameRect.setY(userNameRect.y() + (userNameRect.height() / 2 - nameFontMetrics.height() / 2));
    userNameRect.setHeight(nameFontMetrics.height());
    if (isEntity) {
        userNameRect.setWidth(qMin(nameFontMetrics.width(nameText), optV4.rect.width() - m_avatarSize - (2 * m_spacing)));
    }

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);
    painter->drawText(userNameRect,
                      nameFontMetrics.elidedText(nameText, Qt::ElideRight, userNameRect.width()), textOption);
    painter->restore();

    const Tp::AccountPtr &account = index.data(KTp::AccountRole).value<Tp::AccountPtr>();
    if (isEntity && account) {
        const QPixmap accountIcon = KIcon(account->iconName()).pixmap(m_avatarSize);
        QRect accountIconRect = optV4.rect;
        accountIconRect.adjust(optV4.rect.width() - m_avatarSize - m_spacing, 0, 0, 0);
        style->drawItemPixmap(painter, accountIconRect, 0, accountIcon);
    }
}

QSize EntityViewDelegate::sizeHintContact(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, qMax(m_avatarSize + 2 * m_spacing, KGlobalSettings::smallestReadableFont().pixelSize() + m_spacing));
}


void EntityViewDelegate::paintHeader(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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
    groupRect.adjust(m_spacing, 0, -m_spacing, -1);

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
    const QRect groupLabelRect = groupRect.adjusted(expandSignOption.rect.width() + m_spacing * 2, 0, -m_spacing, 0);
    const QString groupHeaderString =  index.data(Qt::DisplayRole).toString();
    const QFontMetrics groupFontMetrics(groupFont);

    painter->setFont(groupFont);

    painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    painter->drawText(groupLabelRect, Qt::AlignVCenter | Qt::AlignLeft,
                      groupFontMetrics.elidedText(groupHeaderString, Qt::ElideRight,
                                                  groupLabelRect.width() - m_spacing));

    painter->restore();
}

QSize EntityViewDelegate::sizeHintHeader(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    // Add one point to the bottom for the 1px line
    return QSize(0, qMax(m_avatarSize, KGlobalSettings::smallestReadableFont().pixelSize()) + m_spacing + 1);
}






#include "entity-view-delegate.moc"
