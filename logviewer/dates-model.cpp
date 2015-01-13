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

#include "dates-model.h"

#include <KTp/Logger/log-manager.h>
#include <KTp/Logger/log-entity.h>
#include <KTp/Logger/log-search-hit.h>
#include <KTp/Logger/pending-logger-dates.h>
#include <TelepathyQt/Account>

#include <KLocalizedString>
#include <klocalizeddate.h>

Q_DECLARE_METATYPE(Tp::AccountPtr)

typedef QPair<Tp::AccountPtr, KTp::LogEntity> AccountEntityPair;

class Node
{
  public:
    enum Type {
        Root,
        Month,
        Date,
        Account
    };
    Node(Type type, const QDate &date = QDate(), Node *parent = 0);
    virtual ~Node();

    void addChild(Node *child);
    Node* prevSibling() const;
    Node* nextSibling() const;

    Type type;
    QDate date;

    Node* parent;
    QList<Node*> children;
};

class DateNode: public Node
{
  public:
    DateNode(const QDate &date, const KTp::LogEntity &entity, Node *parent = 0);
    virtual ~DateNode();

    KTp::LogEntity entity;
};

class AccountNode: public DateNode
{
  public:
    AccountNode(const QDate &date, const Tp::AccountPtr &account,
                const KTp::LogEntity &entity, Node *parent = 0);
    ~AccountNode();

    Tp::AccountPtr account;
};


class DatesModel::Private
{
  public:
    Private();
    ~Private();

    Node* nodeForDate(const QDate &date, Node *parent);

    QList<KTp::LogSearchHit> searchHits;
    int resetInProgress;

    Node *rootNode;

    QList<KTp::PendingLoggerOperation*> pendingDates;
};

Node::Node(Node::Type type_, const QDate &date_, Node *parent_):
    type(type_),
    date(date_)
{
    if (parent_) {
        parent_->addChild(this);
    }
}

Node::~Node()
{
    qDeleteAll(children);
}

void Node::addChild(Node *child)
{
    children.append(child);
    child->parent = this;
}

Node* Node::nextSibling() const
{
    if (!parent || parent->children.indexOf(const_cast<Node*>(this)) == parent->children.count() - 1) {
        return 0;
    }

    return parent->children.at(parent->children.indexOf(const_cast<Node*>(this)) + 1);
}

Node* Node::prevSibling() const
{
    if (!parent || parent->children.indexOf(const_cast<Node*>(this)) == 0) {
        return 0;
    }

    return parent->children.at(parent->children.indexOf(const_cast<Node*>(this)) - 1);
}

DateNode::DateNode(const QDate &date, const KTp::LogEntity &entity_, Node *parent):
    Node(Node::Date, date, parent),
    entity(entity_)
{
}

DateNode::~DateNode()
{
}

AccountNode::AccountNode(const QDate &date_, const Tp::AccountPtr &account_,
                         const KTp::LogEntity &entity_, Node* parent):
    DateNode(date_, entity_, parent),
    account(account_)
{
    type = Node::Account;
}

AccountNode::~AccountNode()
{
}

DatesModel::Private::Private():
    resetInProgress(0),
    rootNode(new Node(Node::Root))
{
}

DatesModel::Private::~Private()
{
    delete rootNode;
}

Node* DatesModel::Private::nodeForDate(const QDate &date, Node* parent)
{
    Q_FOREACH (Node *node, parent->children) {
        if (node->date == date) {
            return node;
        }
    }

    return 0;
}


bool sortDatesDescending(const Node* date1, const Node* date2)
{
    return date1->date > date2->date;
}

DatesModel::DatesModel(QObject* parent):
    QAbstractItemModel(parent),
    d(new Private)
{
}

DatesModel::~DatesModel()
{
    clear();
    delete d;
}

void DatesModel::addEntity(const Tp::AccountPtr &account, const KTp::LogEntity &entity)
{
    if (d->resetInProgress == 0) {
        beginResetModel();
    }
    ++d->resetInProgress;

    KTp::LogManager *logManager = KTp::LogManager::instance();
    KTp::PendingLoggerDates *pendingDates = logManager->queryDates(account, entity);
    d->pendingDates << pendingDates;
    connect(pendingDates, SIGNAL(finished(KTp::PendingLoggerOperation*)),
            SLOT(onDatesReceived(KTp::PendingLoggerOperation*)));
}

void DatesModel::setEntity(const Tp::AccountPtr &account, const KTp::LogEntity &entity)
{
    clear();
    addEntity(account, entity);
}

void DatesModel::setSearchHits(const QList<KTp::LogSearchHit> &searchHits)
{
    d->searchHits = searchHits;
}

void DatesModel::clearSearchHits()
{
    d->searchHits.clear();
}

void DatesModel::clear()
{
    beginResetModel();
    d->resetInProgress = 0;
    Q_FOREACH (KTp::PendingLoggerOperation *op, d->pendingDates) {
        disconnect(op, SIGNAL(finished(KTp::PendingLoggerOperation*)));
        op->deleteLater();
    }
    d->pendingDates.clear();
    delete d->rootNode;
    d->rootNode = new Node(Node::Root);

    // Don't reset searchHits!
    endResetModel();
}

QDate DatesModel::nextDate(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QDate();
    }

    // Group - it should never be selected
    Node *node = static_cast<Node*>(index.internalPointer());
    if (node->type == Node::Root || node->type == Node::Month) {
        return QDate();
    }

    // Date
    Node *dateNode = (node->type == Node::Date) ? node : node->parent;
    Node *parentNode = dateNode->parent;
    Node *nextDateNode = dateNode->nextSibling();

    if (!nextDateNode) {
        parentNode = parentNode->nextSibling();
        if (!parentNode) {
            return QDate();
        }
        nextDateNode = parentNode->children.first();
    }

    return nextDateNode->date;
}

QDate DatesModel::previousDate(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QDate();
    }

    // Group - it should never be selected
    Node *node = static_cast<Node*>(index.internalPointer());
    if (node->type == Node::Root || node->type == Node::Month) {
        return QDate();
    }

    // Date
    Node *dateNode = (node->type == Node::Date) ? node : node->parent;
    Node *parentNode = dateNode->parent;
    Node *prevDateNode = dateNode->prevSibling();

    if (!prevDateNode) {
        parentNode = parentNode->prevSibling();
        if (!parentNode) {
            return QDate();
        }
        prevDateNode = parentNode->children.last();
    }

    return prevDateNode->date;
}

QModelIndex DatesModel::indexForDate(const QDate &date) const
{
    // FIXME: Hash table?
    const QList<Node*> groups = d->rootNode->children;
    Q_FOREACH (Node *groupNode, groups) {
        const QList<Node*> dates = groupNode->children;
        Q_FOREACH (Node *dateNode, dates) {
            if (dateNode->date == date) {
                return createIndex(groupNode->children.indexOf(dateNode), 0, dateNode);
            }
        }
    }

    return QModelIndex();
}

QVariant DatesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Node *node = static_cast<Node*>(index.internalPointer());
    if (role == DateRole) {
        return node->date;
    }

    // It's a group
    if (node->type == Node::Month) {
        if (role == Qt::DisplayRole) {
            return node->date.toString(QLatin1String("MMMM yyyy"));
        } else if (role == TypeRole) {
            return DatesModel::GroupRow;
        }
    } else if (node->type == Node::Date) {
        if (role == Qt::DisplayRole) {
            return KLocalizedDate(node->date).formatDate();
        } else if (role == TypeRole) {
            return DatesModel::DateRow;
        } else if (role == HintRole) {
            return i18ncp("Number of existing conversations.", "%1 conversation", "%1 conversations", node->children.count());
        } else if (role == EntityRole) {
            return QVariant::fromValue(static_cast<DateNode*>(node)->entity);
        } else if (role == AccountRole) {
            // If there's more than one account, have user pick one
            if (node->children.count() > 1) {
                return QVariant();
            } else {
                return QVariant::fromValue(static_cast<AccountNode*>(node->children.first())->account);
            }
        }
    } else if (node->type == Node::Account) {
        AccountNode *accountNode = static_cast<AccountNode*>(node);
        if (role == Qt::DisplayRole) {
            return accountNode->entity.alias();
        } else if (role == HintRole) {
            return accountNode->account->displayName();
        } else if (role == TypeRole) {
            return ConversationRow;
        } else if (role == AccountRole) {
            return QVariant::fromValue(accountNode->account);
        } else if (role == EntityRole) {
            return QVariant::fromValue(accountNode->entity);
        }
    }

    return QVariant();
}

int DatesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

 int DatesModel::rowCount(const QModelIndex &parent) const
 {
    Node *parentNode = 0;

    // Groups
    if (!parent.isValid()) {
        parentNode = d->rootNode;
    } else {
        parentNode = static_cast<Node*>(parent.internalPointer());
    }

    // Don't make date expandable, if there is only one account
    if (parentNode->type == Node::Date && parentNode->children.count() == 1) {
        return 0;
    }

    return parentNode->children.count();
}

QModelIndex DatesModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    Node *childNode = static_cast<Node*>(child.internalPointer());
    Node *node = childNode->parent;
    if (node == d->rootNode) {
        return QModelIndex();
    }

    Node *parentNode = node->parent;
    return createIndex(parentNode->children.indexOf(node), 0, node);
}

QModelIndex DatesModel::index(int row, int column, const QModelIndex &parent) const
{
    Node *parentNode = 0;

    if (!parent.isValid()) {
        parentNode = d->rootNode;
    } else {
        parentNode = static_cast<Node*>(parent.internalPointer());
    }

    if (row >= parentNode->children.count()) {
        return QModelIndex();
    }

    return createIndex(row, column, parentNode->children.at(row));
}

void DatesModel::onDatesReceived(KTp::PendingLoggerOperation *operation)
{
    // Stop here if clear() was called meanwhile
    if (d->resetInProgress == 0) {
        return;
    }

    KTp::PendingLoggerDates *op = qobject_cast<KTp::PendingLoggerDates*>(operation);
    Q_ASSERT(op);
    Q_ASSERT(d->pendingDates.contains(op));

    d->pendingDates.removeOne(op);
    op->deleteLater();

    QList<QDate> newDates = op->dates();

    Q_FOREACH (const QDate &newDate, newDates) {
        const QDate groupDate(newDate.year(), newDate.month(), 1);
        Node *groupNode = d->nodeForDate(groupDate, d->rootNode);
        if (!groupNode) {
            groupNode = new Node(Node::Month, groupDate, d->rootNode);
            qStableSort(d->rootNode->children.begin(), d->rootNode->children.end(), sortDatesDescending);
        }

        DateNode *dateNode = static_cast<DateNode*>(d->nodeForDate(newDate, groupNode));
        if (!dateNode) {
            dateNode = new DateNode(newDate, op->entity(), groupNode);
            qStableSort(groupNode->children.begin(), groupNode->children.end(), sortDatesDescending);
        }

        if (!op->account().isNull()) {
            new AccountNode(newDate, op->account(), op->entity(), dateNode);
        }
    }

    --d->resetInProgress;
    if (d->resetInProgress == 0) {
        endResetModel();
    }

    if (d->pendingDates.isEmpty()) {
        Q_EMIT datesReceived();
    }
}
