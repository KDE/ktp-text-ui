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

#include "person-entity-merge-model.h"
#include "entity-model.h"

#include <KTp/Logger/log-entity.h>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactManager>

#include <KDE/KDebug>
#include <KDE/KLocalizedString>
#include <KDE/KIconLoader>

class PersonEntityMergeModel::Item
{
  public:
    explicit Item():
        parent(0)
    {
    }

    virtual ~Item()
    {
        qDeleteAll(children);
        children.clear();
    }

    void addChild(Item *item)
    {
        if (item->parent) {
            item->parent->children.removeAll(item);
        }
        children << item;
        item->parent = this;
    }

    Item *parent;
    QList<Item*> children;
};

class PersonEntityMergeModel::GroupItem: public Item
{
  public:
    explicit GroupItem():
        Item()
    {}

    virtual ~GroupItem()
    {
    }

    QString label;
};

class PersonEntityMergeModel::ContactItem: public Item
{
  public:
    explicit ContactItem():
        Item()
    {}

    virtual ~ContactItem()
    {
    }

    bool isPersona() const
    {
        return personaIndex.isValid() && !contactIndex.isValid();
    }

    QPersistentModelIndex personaIndex;
    QPersistentModelIndex contactIndex;
    QPersistentModelIndex entityIndex;

};

PersonEntityMergeModel::ContactItem* PersonEntityMergeModel::itemForPerson(const QModelIndex &contactsModel_personIndex)
{
    if (!contactsModel_personIndex.isValid()) {
        return 0;
    }

    // FIXME: This is slow...
    Q_FOREACH (Item *group, m_rootItem->children) {
        Q_FOREACH (Item *item, group->children) {
            ContactItem *cItem = dynamic_cast<ContactItem*>(item);
            Q_ASSERT(cItem);
            if (cItem->personaIndex == contactsModel_personIndex) {
                kDebug() << "\t\tFound existing persona for" << contactsModel_personIndex.data();
                return cItem;
            }
        }
    }

    kDebug() << "\t\tCreating a new persona for" << contactsModel_personIndex.data();
    ContactItem *item = new ContactItem;
    item->personaIndex = contactsModel_personIndex;

    const QStringList groupNames = contactsModel_personIndex.data(KTp::ContactGroupsRole).toStringList();
    QString groupName;
    if (KTp::kpeopleEnabled()) {
        groupName = groupNames.isEmpty() ? QString() : groupNames.first();
    } else {
        const Tp::AccountPtr account = contactsModel_personIndex.data(KTp::AccountRole).value<Tp::AccountPtr>();
        groupName = account->displayName();
    }

    GroupItem *groupItem = groupForName(groupName);
    const QModelIndex groupIndex = indexForItem(groupItem);
    Q_ASSERT(groupIndex.isValid());
    beginInsertRows(groupIndex, groupItem->children.count(), groupItem->children.count());
    groupItem->addChild(item);
    endInsertRows();

    return item;
}

PersonEntityMergeModel::GroupItem* PersonEntityMergeModel::groupForName(const QString &name)
{
    QString groupName = name;

    if (groupName.isEmpty()) {
        groupName = i18n("Unsorted");
    }

    Q_FOREACH (Item *item, m_rootItem->children) {
        GroupItem *group = dynamic_cast<GroupItem*>(item);
        Q_ASSERT(group);

        if (group->label == groupName) {
            kDebug() << "\tFound matching group" << groupName;
            return group;
        }
    }

    kDebug() << "\tCreating a new group" << groupName;
    GroupItem *group = new GroupItem;
    group->label = groupName;

    beginInsertRows(QModelIndex(), m_rootItem->children.count(), m_rootItem->children.count());
    m_rootItem->addChild(group);
    endInsertRows();

    return group;
}

PersonEntityMergeModel::Item* PersonEntityMergeModel::itemForMergeModelIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_rootItem;
    }

    Q_ASSERT(index.model() == this);

    Item *parent = static_cast<Item*>(index.internalPointer());
    Q_ASSERT(parent != 0);

    Q_ASSERT(index.row() < parent->children.count());
    Item *item = parent->children.at(index.row());
    Q_ASSERT(item != 0);

    return item;
}

PersonEntityMergeModel::Item* PersonEntityMergeModel::itemForEntityModelIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_rootItem;
    }

    Q_ASSERT(index.model() == m_entityModel);

    const QString id = index.data(EntityModel::IdRole).toString();
    ContactItem *item = static_cast<ContactItem*>(m_contactLookup.value(id));
    Q_ASSERT(item);

    return item;
}

PersonEntityMergeModel::Item* PersonEntityMergeModel::itemForContactsModelIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_rootItem;
    }

    Q_ASSERT(index.model() == m_contactsModel);

    KTp::RowType rowType = static_cast<KTp::RowType>(index.data(KTp::RowTypeRole).toInt());
    if (rowType == KTp::GroupRowType) {
        Q_FOREACH (Item *item, m_rootItem->children) {
            GroupItem *group = dynamic_cast<GroupItem*>(item);
            if (group->label == index.data(Qt::DisplayRole).toString()) {
                return group;
            }
        }
        return 0;
    } else if (rowType == KTp::PersonRowType) {
        Q_FOREACH (Item *item, m_contactLookup) {
            ContactItem *parent = dynamic_cast<ContactItem*>(item->parent);
            if (parent && parent->personaIndex == index) {
                return parent;
            }
        }

        return 0;
    } else if (rowType == KTp::ContactRowType) {
        const QString id = index.data(KTp::IdRole).toString();
        return m_contactLookup.value(id);
    }

    Q_ASSERT(false && "Invalid rowType");
    return 0;
}


PersonEntityMergeModel::PersonEntityMergeModel(KTp::ContactsModel *contactsModel,
                                               EntityModel *entityModel,
                                               QObject *parent):
    QAbstractItemModel(parent),
    m_contactsModel(contactsModel),
    m_entityModel(entityModel),
    m_rootItem(new ContactItem)
{
    connect(m_contactsModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(contactsModelRowsInserted(QModelIndex,int,int)));
    connect(m_contactsModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(contactsModelRowsRemoved(QModelIndex,int,int)));
    connect(m_contactsModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(contactsModelDataChanged(QModelIndex,QModelIndex)));

    connect(m_entityModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(entityModelRowsInserted(QModelIndex,int,int)));
    connect(m_entityModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(entityModelRowsRemoved(QModelIndex,int,int)));
    connect(m_entityModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(entityModelDataChanged(QModelIndex,QModelIndex)));
}

PersonEntityMergeModel::~PersonEntityMergeModel()
{
    delete m_rootItem;
}

int PersonEntityMergeModel::rowCount(const QModelIndex& parent) const
{
    Item *parentItem = itemForMergeModelIndex(parent);
    return parentItem->children.count();
}

int PersonEntityMergeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 1;
}

Qt::ItemFlags PersonEntityMergeModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)

    // TODO: Make Persons selectable and show some fancy stuff
    /*
    if (index.data(PersonsModel::ResourceTypeRole).toUInt() == PersonsModel::Person) {
        return Qt::ItemIsEnabled;
    }
    */

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PersonEntityMergeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    // If parent is m_rootItem, then we are dealing with a group
    if (index.internalPointer() == m_rootItem) {
        GroupItem *item = dynamic_cast<GroupItem*>(itemForMergeModelIndex(index));
        Q_ASSERT(item);

        if (role == Qt::DisplayRole) {
            return item->label;
        } else if (role == PersonEntityMergeModel::ItemTypeRole) {
            return PersonEntityMergeModel::Group;
        }

        // FIXME: We could support more roles?
        return QVariant();
    }

    // Now we are sure it's either persona or contact
    ContactItem *item = dynamic_cast<ContactItem*>(itemForMergeModelIndex(index));
    Q_ASSERT(item);

    switch (role) {
        case PersonEntityMergeModel::EntityRole:
            return item->entityIndex.data(EntityModel::EntityRole);
        case PersonEntityMergeModel::ContactRole:
            return item->personaIndex.data(KTp::ContactRole);
        case PersonEntityMergeModel::AccountRole:
            return item->entityIndex.data(EntityModel::AccountRole);
        case PersonEntityMergeModel::ItemTypeRole:
            return item->isPersona() ? PersonEntityMergeModel::Persona
                                     : PersonEntityMergeModel::Entity;
    }

    // Extract role from respective parent model
    QVariant value;
    if (item->isPersona()) {
        value = m_contactsModel->data(item->personaIndex, role);
    } else if (item->contactIndex.isValid()) {
        value = m_contactsModel->data(item->contactIndex, role);
        if (value.isNull()) {
            value = m_entityModel->data(item->entityIndex, role);
        }
    } else {
        value = m_entityModel->data(item->entityIndex, role);
    }

    // Use the blue pawn avatar if none is provided
    if (role == Qt::DecorationRole && value.isNull()) {
        value = KIconLoader::global()->loadIcon(QLatin1String("im-user"), KIconLoader::NoGroup, KIconLoader::SizeSmall);
    }

    return value;
}

QModelIndex PersonEntityMergeModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column, itemForMergeModelIndex(parent));
}

QModelIndex PersonEntityMergeModel::parent(const QModelIndex& child) const
{
    Item *parent = static_cast<Item*>(child.internalPointer());
    if (!parent->parent) {
        return QModelIndex();
    }

    Item *parentParent = parent->parent;
    return createIndex(parentParent->children.indexOf(parent), 0, parentParent);
}

void PersonEntityMergeModel::findPersonForId(const QString &entityId, PersonEntityMergeModel::Item **parentItem,
                                             QModelIndex &personaIndex, QModelIndex &contactIndex)
{
    for (int j = 0; j < m_contactsModel->rowCount(); ++j) {
        const QModelIndex index = m_contactsModel->index(j, 0);
        const KTp::RowType rowType = static_cast<KTp::RowType>(index.data(KTp::RowTypeRole).toInt());
        if (rowType == KTp::ContactRowType) {
            if (index.data(KTp::IdRole).toString() == entityId) {
                *parentItem = itemForPerson(index);
                contactIndex = index;
                return;
            }
        } else if (rowType == KTp::PersonRowType) {
            for (int k = 0; k < m_contactsModel->rowCount(index); ++k) {
                const QModelIndex childIndex = m_contactsModel->index(k, 0, index);
                if (m_contactsModel->data(childIndex, KTp::IdRole).toString() == entityId) {
                    //kDebug() << "\tFound matching persona" << m_personsModel->data(index, PersonsModel::UriRole).toString();
                    //kDebug() << "\t\tFound matching contact" << m_personsModel->data(childIndex, PersonsModel::IMRole).toString();
                    *parentItem = itemForPerson(index);
                    personaIndex = index;
                    contactIndex = childIndex;
                    return;
                }
            }
        }
    }

    *parentItem = 0;
    personaIndex = QModelIndex();
    contactIndex = QModelIndex();
}

QModelIndex PersonEntityMergeModel::indexForItem(PersonEntityMergeModel::Item* item) const
{
    if (item == m_rootItem) {
        return QModelIndex();
    }

    Item *parent = item->parent;
    if (parent == m_rootItem) {
        return index(parent->children.indexOf(item), 0);
    } else if (parent->parent == m_rootItem) {
        const QModelIndex parentIndex = indexForItem(parent);
        return index(parent->children.indexOf(item), 0, parentIndex);
    } else if (parent->parent->parent == m_rootItem) {
        const QModelIndex parentIndex = indexForItem(parent->parent);
        return index(parent->children.indexOf(item), 0, parentIndex);
    }

    return QModelIndex();
}

void PersonEntityMergeModel::entityModelRowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; i++) {
        const QModelIndex rowIndex = m_entityModel->index(i, 0, parent);
        Q_ASSERT(rowIndex.isValid());
        QModelIndex contactIndex;
        QModelIndex personaIndex;
        Item *parentItem = 0;

        const QString id = rowIndex.data(EntityModel::IdRole).toString();
        findPersonForId(id, &parentItem, personaIndex, contactIndex);

        if (!contactIndex.isValid() && !personaIndex.isValid()) {
            QString groupName;
            if (!KTp::kpeopleEnabled()) {
                const Tp::AccountPtr account = rowIndex.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
                groupName = account->displayName();
            }
            parentItem = groupForName(groupName);
        }

        const QModelIndex parentIndex = indexForItem(parentItem);
        Q_ASSERT(parentIndex.isValid());

        ContactItem *item = new ContactItem;
        item->entityIndex = rowIndex;
        item->contactIndex = contactIndex;
        item->personaIndex = personaIndex;

        beginInsertRows(parentIndex, parentItem->children.count(), parentItem->children.count());
        parentItem->addChild(item);
        m_contactLookup.insert(id, item);
        endInsertRows();
    }
}

void PersonEntityMergeModel::entityModelRowsRemoved(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; i++) {
        const QModelIndex entityModelIndex = m_contactsModel->index(start, 0, parent);
        Item *item = itemForEntityModelIndex(entityModelIndex);
        const int pos = item->parent->children.indexOf(item);
        beginRemoveRows(indexForItem(item), pos, pos);
        delete item->parent->children.takeAt(pos);
        endRemoveRows();
    }
}

void PersonEntityMergeModel::entityModelDataChanged(const QModelIndex &topLeft,
                                                    const QModelIndex &bottomRight)
{
    // EntityModel always notifies about single item change
    Q_ASSERT(bottomRight == topLeft);
    Q_UNUSED(bottomRight);

    Item *item = itemForEntityModelIndex(topLeft);
    const QModelIndex changedIndex = indexForItem(item);
    Q_EMIT dataChanged(changedIndex, changedIndex);
}

void PersonEntityMergeModel::contactsModelRowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; i++) {
        const QModelIndex personaIndex = m_contactsModel->index(i, 0, parent);
        Q_ASSERT(personaIndex.isValid());

        const QString id = personaIndex.data(KTp::IdRole).toString();
        Item *entityItem = m_contactLookup.value(id);
        // We don't (yet) have such entity available
        if (!entityItem) {
            continue;
        }

        ContactItem *contactItem = itemForPerson(personaIndex);
        Q_ASSERT(contactItem);

        const QModelIndex entityIndex = indexForItem(entityItem);
        const QModelIndex parentIndex = indexForItem(contactItem);
        beginMoveRows(entityIndex.parent(), entityIndex.row(), entityIndex.row(),
                      parentIndex, contactItem->children.count());
        contactItem->addChild(entityItem);
        endMoveRows();
    }
}

void PersonEntityMergeModel::contactsModelRowsRemoved(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i <= end; i++) {
        const QModelIndex personIndex = m_contactsModel->index(i, 0, parent);
        KTp::RowType type = static_cast<KTp::RowType>(personIndex.data(KTp::RowTypeRole).toInt());
        if (type == KTp::GroupRowType) {
            // ignore group removal
        } else if (type == KTp::PersonRowType) {
            Item *item = itemForContactsModelIndex(personIndex);
            if (!item) {
                return;
            }
            // Move all contacts in given person to "Unsorted" group
            const QModelIndex index = indexForItem(item);
            GroupItem *group = groupForName(QString());
            const QModelIndex groupIndex = indexForItem(group);
            beginMoveRows(index, 0, item->children.count(), groupIndex, group->children.count());
            while (!item->children.isEmpty()) {
                group->addChild(item->children.at(0));
            }
            endMoveRows();

            group = dynamic_cast<GroupItem*>(item->parent);
            const int pos = group->children.indexOf(item);
            beginRemoveRows(index.parent(), pos, pos);
            delete group->children.takeAt(pos);
            endRemoveRows();
        } else if (type == KTp::ContactRowType) {
            // ignore contact removal
        }
    }
}

void PersonEntityMergeModel::contactsModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
        const QModelIndex row = m_contactsModel->index(i, 0, topLeft.parent());

        Item *item = itemForContactsModelIndex(row);
        if (!item) {
            return;
        }

        const QModelIndex index = indexForItem(item);

        Q_EMIT dataChanged(index, index);
    }
}

#include "person-entity-merge-model.moc"
