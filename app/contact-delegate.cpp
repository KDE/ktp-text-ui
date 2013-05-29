/*
 * Contact Delegate
 *
 * Copyright (C) 2010-2011 Collabora Ltd. <info@collabora.co.uk>
 *   @Author Dario Freddi <dario.freddi@collabora.co.uk>
 * Copyright (C) 2011 Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2012 Dominik Cermak <d.cermak@arcor.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "contact-delegate.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QApplication>
#include <QStyle>

#include <KIconLoader>
#include <KIcon>
#include <KDebug>
#include <KGlobalSettings>
#include <KDE/KLocale>

#include <KTp/types.h>
#include <KTp/presence.h>

ContactDelegate::ContactDelegate(QObject * parent)
    : AbstractContactDelegate(parent)
    , m_avatarSize(IconSize(KIconLoader::Dialog))
    , m_presenceIconSize(IconSize(KIconLoader::Toolbar))
    , m_spacing(IconSize(KIconLoader::Dialog) / 8)
{

}

ContactDelegate::~ContactDelegate()
{

}

void ContactDelegate::paintContact(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 optV4 = option;
    initStyleOption(&optV4, index);

    painter->save();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter->setClipRect(optV4.rect);

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect iconRect = optV4.rect;
    iconRect.setSize(QSize(m_avatarSize, m_avatarSize));
    iconRect.moveTo(QPoint(iconRect.x() + m_spacing, iconRect.y() + m_spacing));

    QPixmap avatar(qvariant_cast<QPixmap>(index.data(KTp::ContactAvatarPixmapRole)));
    if (index.data(KTp::ContactUnreadMessageCountRole).toInt() > 0) {
        avatar = SmallIcon(QLatin1String("mail-unread-new"), KIconLoader::SizeMedium);
    }

    QPainterPath roundedPath;
    roundedPath.addRoundedRect(iconRect, 20, 20, Qt::RelativeSize);

    painter->save();
    painter->setClipPath(roundedPath);

    style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, avatar.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    painter->restore();

    QPen thinLinePen;
    thinLinePen.setWidth(0);
    thinLinePen.setColor(option.palette.color(QPalette::Disabled, QPalette::Text));

    painter->save();

    painter->setPen(thinLinePen);
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawPath(roundedPath);

    //clear the font and AA setting
    painter->restore();

    // This value is used to set the correct width for the username and the presence message.
    int rightIconsWidth = m_presenceIconSize + m_spacing;

    QPixmap icon = KIcon(index.data(KTp::ContactPresenceIconRole).toString()).pixmap(KIconLoader::SizeSmallMedium);

    QRect statusIconRect = optV4.rect;
    statusIconRect.setSize(QSize(m_presenceIconSize, m_presenceIconSize));
    statusIconRect.moveTo(QPoint(optV4.rect.right() - rightIconsWidth,
                                 optV4.rect.top() + (optV4.rect.height() - m_presenceIconSize) / 2));

    painter->drawPixmap(statusIconRect, icon);

    // Right now we only check for 'phone', as that's the most interesting type.
    if (index.data(KTp::ContactClientTypesRole).toStringList().contains(QLatin1String("phone"))) {
        // Additional space is needed for the icons, don't add too much spacing between the two icons
        rightIconsWidth += m_presenceIconSize + m_spacing / 2;

        QPixmap phone = QIcon::fromTheme(QLatin1String("phone")).pixmap(m_presenceIconSize);
        QRect phoneIconRect = optV4.rect;
        phoneIconRect.setSize(QSize(m_presenceIconSize, m_presenceIconSize));
        phoneIconRect.moveTo(QPoint(optV4.rect.right() - rightIconsWidth,
                                    optV4.rect.top() + (optV4.rect.height() - m_presenceIconSize) / 2));
        painter->drawPixmap(phoneIconRect, phone);
    }

    QRect userNameRect = optV4.rect;
    userNameRect.setX(iconRect.x() + iconRect.width() + m_spacing);
    userNameRect.setY(userNameRect.y() + m_spacing / 2);
    userNameRect.setWidth(userNameRect.width() - rightIconsWidth);

    const QFontMetrics nameFontMetrics(KGlobalSettings::generalFont());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
    }

    painter->drawText(userNameRect,
                      nameFontMetrics.elidedText(optV4.text, Qt::ElideRight, userNameRect.width()));

    const QFontMetrics statusFontMetrics(KGlobalSettings::smallestReadableFont());

    QRect statusMsgRect = optV4.rect;
    statusMsgRect.setX(iconRect.x() + iconRect.width() + m_spacing);
    statusMsgRect.setY(userNameRect.bottom() - statusFontMetrics.height() - 4);
    statusMsgRect.setWidth(statusMsgRect.width() - rightIconsWidth);

    QColor fadingColor;
    if (option.state & QStyle::State_Selected) {
        fadingColor = QColor(option.palette.color(QPalette::Disabled, QPalette::HighlightedText));
    } else {
        fadingColor = QColor(option.palette.color(QPalette::Disabled, QPalette::Text));
    }

    painter->setPen(fadingColor);

    painter->setFont(KGlobalSettings::smallestReadableFont());
    painter->drawText(statusMsgRect,
                      statusFontMetrics.elidedText(index.data(KTp::ContactLastMessageRole).toString().trimmed(),
                                                   Qt::ElideRight, statusMsgRect.width()));

    painter->restore();
}

QSize ContactDelegate::sizeHintContact(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(0, m_avatarSize + 2 * m_spacing);
}

#include "contact-delegate.moc"
