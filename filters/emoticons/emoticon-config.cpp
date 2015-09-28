/*
 * Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "emoticon-config.h"

#include <QLabel>
#include <QComboBox>
#include <QInputDialog>
#include <QListWidgetItem>

#include <KEmoticons>
#include <KEmoticonsTheme>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KWidgetItemDelegate>

#include <KTp/core.h>
#include <TelepathyQt/AccountSet>
#include <TelepathyQt/AccountManager>

K_PLUGIN_FACTORY_WITH_JSON(EmoticonConfigFactory,
                           "kcm_ktptextui_message_filter_emoticons.json",
                           registerPlugin<EmoticonFilterConfig>();)

enum {
    AccountNameRole = Qt::UserRole + 10,
    AccountIdRole = Qt::UserRole + 11,
    EmoticonsThemeRole = Qt::UserRole + 12
};

static QIcon previewEmoticon(const KEmoticonsTheme &theme)
{
    QString path = theme.tokenize(QStringLiteral(":)"))[0].picPath;
    if (path.isEmpty()) {
        path = theme.emoticonsMap().keys().value(0);
    }
    return QIcon(path);
}

class ItemDelegate : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    explicit ItemDelegate(QAbstractItemView *itemView, QObject *parent = Q_NULLPTR)
        : KWidgetItemDelegate(itemView, parent)
        , m_comboBox(new QComboBox())
    {
        // For size calculation in sizeHint()
        m_comboBox->addItem(previewEmoticon(m_emoticons.theme()), QStringLiteral("name"));
    }

    QList<QWidget*> createItemWidgets(const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        Q_UNUSED(index);

        QComboBox *comboBox = new QComboBox();
        connect(comboBox, &QComboBox::currentTextChanged, this, &ItemDelegate::comboBoxCurrentTextChanged);

        for (const QString &name : m_emoticons.themeList()) {
            KEmoticonsTheme theme = m_emoticons.theme(name);
            comboBox->addItem(previewEmoticon(theme), theme.themeName());
        }

        return {comboBox};
    }

    void updateItemWidgets(const QList<QWidget*> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const Q_DECL_OVERRIDE
    {
        const int margin = option.fontMetrics.height() / 2;
        QComboBox *comboBox = static_cast<QComboBox*>(widgets.at(0));
        comboBox->move((option.rect.width() + margin) / 2, margin);
        comboBox->resize(option.rect.width() / 2 - margin, comboBox->sizeHint().height());
        comboBox->setCurrentText(index.data(EmoticonsThemeRole).toString());
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        QStyle *style = itemView()->style();
        const int margin = option.fontMetrics.height() / 2;
        const QPalette::ColorRole colorRole = option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text;

        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

        QRect textRect = option.rect;
        textRect.setWidth(textRect.width() / 2 - margin);
        textRect.setX(textRect.x() + margin);
        const QString text = elidedText(option.fontMetrics, textRect.width(), Qt::ElideRight, index.data(AccountNameRole).toString());
        style->drawItemText(painter, textRect, Qt::AlignLeft | Qt::AlignVCenter, option.palette, true, text, colorRole);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        Q_UNUSED(index);
        Q_UNUSED(option);

        const int margin = option.fontMetrics.height() / 2;
        QSize size;
        size.setWidth(m_comboBox->sizeHint().width() * 2 + 3 * margin);
        size.setHeight(m_comboBox->sizeHint().height() + 2 * margin);
        return size;
    }

Q_SIGNALS:
    void dataChanged(const QModelIndex &index, int role, const QVariant &data);

private Q_SLOTS:
    void comboBoxCurrentTextChanged(const QString &themeName)
    {
        Q_EMIT dataChanged(focusedIndex(), EmoticonsThemeRole, themeName);
    }

private:
    QComboBox *m_comboBox;
    KEmoticons m_emoticons;
};

EmoticonFilterConfig::EmoticonFilterConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_config(KSharedConfig::openConfig(QStringLiteral("ktp-text-uirc")))
{
    ui.setupUi(this);

    ItemDelegate *delegate = new ItemDelegate(ui.listWidget, this);
    ui.listWidget->setItemDelegate(delegate);

    connect(delegate, &ItemDelegate::dataChanged, this, &EmoticonFilterConfig::dataChanged);
    connect(ui.addBtn, &QPushButton::clicked, this, &EmoticonFilterConfig::addClicked);
    connect(ui.removeBtn, &QPushButton::clicked, this, &EmoticonFilterConfig::removeClicked);
    connect(ui.listWidget, &QListWidget::currentItemChanged, this, &EmoticonFilterConfig::updateButtons);
    connect(ui.listWidget, &QListWidget::itemSelectionChanged, this, &EmoticonFilterConfig::updateButtons);
}

void EmoticonFilterConfig::load()
{
    for (const Tp::AccountPtr &account : KTp::accountManager()->validAccounts()->accounts()) {
        const QString name = account->normalizedName().isEmpty() ? account->displayName() : account->normalizedName();
        m_accounts[account->uniqueIdentifier()] = name;
    }

    KConfigGroup group = m_config->group("Filter-Emoticons");
    for (const QString &key : group.keyList()) {
        if (!m_accounts.contains(key)) {
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(AccountIdRole, key);
        item->setData(AccountNameRole, m_accounts.value(key));
        item->setData(EmoticonsThemeRole, group.readEntry<QString>(key, QString()));
        ui.listWidget->addItem(item);
    }

    updateButtons();
}

void EmoticonFilterConfig::save()
{
    KConfigGroup group = m_config->group("Filter-Emoticons");
    group.deleteGroup();

    for (int i = 0; i < ui.listWidget->count(); ++i) {
        QListWidgetItem *item = ui.listWidget->item(i);
        group.writeEntry<QString>(item->data(AccountIdRole).toString(), item->data(EmoticonsThemeRole).toString());
    }

    m_config->sync();
}

void EmoticonFilterConfig::defaults()
{
    ui.listWidget->clear();
    load();
}

void EmoticonFilterConfig::addClicked()
{
    const QString account = QInputDialog::getItem(
        this, i18nc("@title:window", "Add account"),
        i18nc("@label:listbox", "Select account:"), accountsNotInList(), 0, false);
    if (account.isEmpty()) {
        return;
    }

    QListWidgetItem *item = new QListWidgetItem();
    item->setData(AccountIdRole, m_accounts.key(account));
    item->setData(AccountNameRole, account);
    item->setData(EmoticonsThemeRole, KEmoticons::currentThemeName());
    ui.listWidget->addItem(item);

    updateButtons();
    Q_EMIT changed();
}

void EmoticonFilterConfig::removeClicked()
{
    QListWidgetItem *item = ui.listWidget->currentItem();
    if (!item) {
        return;
    }

    delete item;
    updateButtons();
    Q_EMIT changed();
}

void EmoticonFilterConfig::dataChanged(const QModelIndex &index, int role, const QVariant &data)
{
    QListWidgetItem *item = ui.listWidget->item(index.row());
    if (!item) {
        return;
    }

    item->setData(role, data);
    Q_EMIT changed();
}

void EmoticonFilterConfig::updateButtons()
{
    ui.addBtn->setEnabled(!accountsNotInList().isEmpty());
    ui.removeBtn->setEnabled(!ui.listWidget->selectedItems().isEmpty());
}

QStringList EmoticonFilterConfig::accountsNotInList() const
{
    QStringList names = m_accounts.values();
    for (int i = 0; i < ui.listWidget->count(); ++i) {
        QListWidgetItem *item = ui.listWidget->item(i);
        names.removeOne(item->data(AccountNameRole).toString());
    }
    return names;
}

#include "emoticon-config.moc"
