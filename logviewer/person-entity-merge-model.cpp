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
#include <KTp/Models/contacts-model.h>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactManager>

#include <KDE/KDebug>
#include <KDE/KLocalizedString>
#include <KDE/KIconLoader>

#include <QPixmap>

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
    Tp::AccountPtr account; // when grouping by accounts
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

void PersonEntityMergeModel::addItem(PersonEntityMergeModel::Item* item, PersonEntityMergeModel::Item* parent)
{
    const int pos = parent->children.count();
    beginInsertRows(indexForItem(parent), pos, pos);
    parent->addChild(item);
    endInsertRows();
}

QModelIndex PersonEntityMergeModel::indexForItem(PersonEntityMergeModel::Item *item) const
{
    if (item == m_rootItem) {
        return QModelIndex();
    }

    return index(item->parent->children.indexOf(item), 0, indexForItem(item->parent));
}

PersonEntityMergeModel::ContactItem* PersonEntityMergeModel::itemForPersona(const QModelIndex &personsModel_personaIndex)
{
    if (!personsModel_personaIndex.isValid()) {
        return 0;
    }

    // FIXME: This is slow...
    Q_FOREACH (Item *group, m_rootItem->children) {
        Q_FOREACH (Item *item, group->children) {
            ContactItem *cItem = dynamic_cast<ContactItem*>(item);
            Q_ASSERT(cItem);
            if (cItem->personaIndex == personsModel_personaIndex) {
                kDebug() << "\t\tFound existing persona for" << personsModel_personaIndex.data();
                return cItem;
            }
        }
    }

    kDebug() << "\t\tCreating a new persona for" << personsModel_personaIndex.data();
    ContactItem *item = new ContactItem;
    item->personaIndex = personsModel_personaIndex;

    const QStringList groupNames = personsModel_personaIndex.data(KTp::ContactGroupsRole).toStringList();
    GroupItem *groupItem = groupForName(groupNames.size() ? groupNames.first() : QString());
    addItem(item, groupItem);

    return item;
}

PersonEntityMergeModel::GroupItem* PersonEntityMergeModel::groupForName(const QVariant& data)
{
    QString groupName;

    if (data.canConvert<QVariantList>()) {
        const QVariantList list = data.toList();
        if (!list.isEmpty()) {
            // FIXME: How is it with multi-group membership?
            groupName = list.first().toString();
        }
    } else {
        groupName = data.toString();
    }

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
    addItem(group, m_rootItem);

    return group;
}

PersonEntityMergeModel::Item* PersonEntityMergeModel::itemForIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_rootItem;
    }

    if (index.model() == this) {
        Item *parent = static_cast<Item*>(index.internalPointer());
        Q_ASSERT(parent != 0);

        Q_ASSERT(index.row() < parent->children.count());
        Item *item = parent->children.at(index.row());
        Q_ASSERT(item != 0);

        return item;
    } else if (index.model() == m_entityModel) {
        const QString id = index.data(EntityModel::IdRole).toString();
        // FIXME: We should create a faster lookup method, like having a QHash<QString /*id*/,Item*>
        Q_FOREACH (Item *group, m_rootItem->children) {
            Q_FOREACH (Item *item, group->children) {
                ContactItem *contactItem = static_cast<ContactItem*>(item);
                if (contactItem->isPersona()) {
                    Q_FOREACH (Item *subItem, contactItem->children) {
                        ContactItem *realContact = static_cast<ContactItem*>(subItem);
                        if (realContact->entityIndex.data(EntityModel::IdRole).toString() == id) {
                            return realContact;
                        }
                    }
                } else {
                    if (contactItem->entityIndex.data(EntityModel::IdRole).toString() == id) {
                        return contactItem;
                    }
                }
            }
        }

        Q_ASSERT(false);
        return 0;
    }

    kDebug() << index;
    Q_ASSERT(false && "Invalid index model");
    return 0;
}

PersonEntityMergeModel::PersonEntityMergeModel(KTp::ContactsModel* contactsModel,
                                               EntityModel* entityModel,
                                               QObject* parent):
    QAbstractItemModel(parent),
    m_contactsModel(contactsModel),
    m_entityModel(entityModel),
    m_rootItem(new ContactItem),
    m_initializedSources(0)
{
    connect(m_contactsModel, SIGNAL(modelInitialized(bool)),
                this, SLOT(sourceModelInitialized()));
    connect(m_entityModel, SIGNAL(modelInitialized()),
            this, SLOT(sourceModelInitialized()));
}

PersonEntityMergeModel::~PersonEntityMergeModel()
{
    delete m_rootItem;
}

int PersonEntityMergeModel::rowCount(const QModelIndex& parent) const
{
    Item *parentItem = itemForIndex(parent);
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
        GroupItem *item = dynamic_cast<GroupItem*>(itemForIndex(index));
        Q_ASSERT(item);

        if (role == Qt::DisplayRole) {
            return item->label;
        } else if (role == PersonEntityMergeModel::ItemTypeRole) {
            return PersonEntityMergeModel::Group;
        } else if (role == PersonEntityMergeModel::AccountRole) {
            return qVariantFromValue<Tp::AccountPtr>(item->account);
        }

        // FIXME: We could support more roles?
        return QVariant();
    }


    ContactItem *item = static_cast<ContactItem*>(itemForIndex(index));
    Q_ASSERT(item);

    switch (role) {
        case PersonEntityMergeModel::EntityRole:
            return item->entityIndex.data(EntityModel::EntityRole);
        case PersonEntityMergeModel::ContactRole:
            return item->contactIndex.data(KTp::ContactRole);
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

QModelIndex PersonEntityMergeModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column, itemForIndex(parent));
}

QModelIndex PersonEntityMergeModel::parent(const QModelIndex &child) const
{
    Item *parent = static_cast<Item*>(child.internalPointer());
    if (!parent->parent) {
        return QModelIndex();
    }

    Item *parentParent = parent->parent;
    return createIndex(parentParent->children.indexOf(parent), 0, parentParent);
}

bool PersonEntityMergeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(count); // count == 1

    const QModelIndex itemIndex = index(row, 0, parent);
    Item *item = itemForIndex(itemIndex);
    Q_ASSERT(item);
    if (!item) {
        return false;
    }

    if (item->parent == m_rootItem || static_cast<ContactItem*>(item)->isPersona()) {
        // Removing account or persona
        beginRemoveRows(itemIndex, 0, rowCount(itemIndex));
        while (!item->children.isEmpty()) {
            ContactItem *child = static_cast<ContactItem*>(item->children.takeFirst());
            m_entityModel->removeRows(child->entityIndex.row(), 1, child->entityIndex.parent());
            delete child;
        }
        endRemoveRows();

        beginRemoveRows(parent, row, row);
        delete item->parent->children.takeAt(row);
        endRemoveRows();
    } else {
        ContactItem *contactItem = static_cast<ContactItem*>(item);
        // If there's only one contact, remove the entire group
        if (rowCount(parent) == 1) {
            removeRows(parent.row(), 1, parent.parent());
        } else {
            if (!contactItem->entityIndex.isValid()) {
                // ??
                return false;
            }
            beginRemoveRows(parent, row, row);
            m_entityModel->removeRows(contactItem->entityIndex.row(), 1,
                                    contactItem->entityIndex.parent());
            delete contactItem->parent->children.takeAt(row);
            endRemoveRows();
        }
    }

    return true;
}


void PersonEntityMergeModel::sourceModelInitialized()
{
    Q_ASSERT(m_initializedSources <= 2);

    m_initializedSources++;
    if (m_initializedSources == 2) {
        initializeModel();
        // Don't listen to change notifications until we have populated the model
        // TODO: Listen to changes from KPeople too
        connect(m_entityModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(entityModelDataChanged(QModelIndex,QModelIndex)));

        Q_EMIT modelInitialized();
    }
}


void PersonEntityMergeModel::initializeModel()
{
    kDebug();

    for (int i = 0; i < m_entityModel->rowCount(QModelIndex()); ++i) {
        const QModelIndex entityIndex = m_entityModel->index(i, 0);
        QModelIndex contactIndex;
        QModelIndex personaIndex;
        Item *parentItem = 0;

        const KTp::LogEntity entity = entityIndex.data(EntityModel::EntityRole).value<KTp::LogEntity>();
        const Tp::AccountPtr account = entityIndex.data(EntityModel::AccountRole).value<Tp::AccountPtr>();
        kDebug() << "Searching for match for entity" << entity.id() << "@" << account->uniqueIdentifier();
        if (KTp::kpeopleEnabled()) {
            for (int j = 0; j < m_contactsModel->rowCount(); ++j) {
                const QModelIndex index = m_contactsModel->index(j, 0);
                bool found = false;
                for (int k = 0; k < m_contactsModel->rowCount(index); ++k) {
                    const QModelIndex childIndex = m_contactsModel->index(k, 0, index);
                    if (m_contactsModel->data(childIndex, KTp::IdRole).toString() == entity.id()) {
                        //kDebug() << "\tFound matching persona" << m_personsModel->data(index, PersonsModel::UriRole).toString();
                        //kDebug() << "\t\tFound matching contact" << m_personsModel->data(childIndex, PersonsModel::IMRole).toString();
                        parentItem = itemForPersona(index);
                        personaIndex = index;
                        contactIndex = childIndex;
                        found = true;
                        break;
                    }
                }

                if (found) {
                    break;
                }
            }
        }

        if (!contactIndex.isValid() && !personaIndex.isValid()) {
            kDebug() << "\tNo match";
            // If we don't have kpeople, we don't have information about group
            // membership, so fallback to grouping by account
            if (KTp::kpeopleEnabled()) {
                parentItem = groupForName(QString());
            } else {
                parentItem = groupForName(account->displayName());
                static_cast<GroupItem*>(parentItem)->account = account;
            }
        }

        ContactItem *item = new ContactItem;
        item->entityIndex = entityIndex;
        item->contactIndex = contactIndex;
        item->personaIndex = personaIndex;
        addItem(item, parentItem);
    }
}

void PersonEntityMergeModel::entityModelDataChanged(const QModelIndex &topLeft,
                                                    const QModelIndex &bottomRight)
{
    // EntityModel always notified about single item change
    Q_ASSERT(bottomRight == topLeft);
    Q_UNUSED(bottomRight);

    Item *item = itemForIndex(topLeft);
    Item *parent = item->parent;

    const QModelIndex changedIndex = index(parent->children.indexOf(item), 0);
    Q_EMIT dataChanged(changedIndex, changedIndex);
}



#include "person-entity-merge-model.moc"
