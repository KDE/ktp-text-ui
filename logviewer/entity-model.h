#ifndef ENTITYMODEL_H
#define ENTITYMODEL_H

#include <QAbstractListModel>

#include <TelepathyQt/Types>

#include <TelepathyLoggerQt4/Entity>
#include <TelepathyQt/Account>


/**
    Lists all avilable entities.

    roles:
      - Qt::DisplayRole - name
      - Qt::DecorationRole - avatar
      - EntityModel::IdRole
      - EntityModel::TypeRole - EntityType (EntityTypeContact/Room/Self/Unknown)
      - EntityModel::EntityRole - relevant Tpl::EntityPtr
  */

namespace Tpl{
    class PendingOperation;
}


class EntityModelItem {
public:
    Tpl::EntityPtr entity;
    Tp::AccountPtr account;
};

class EntityModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        IdRole = Qt::UserRole,
        TypeRole,
        EntityRole,
        AccountRole
    };


    explicit EntityModel(QObject *parent = 0);
    void setAccountManager(const Tp::AccountManagerPtr &accountManager);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

Q_SIGNALS:

public Q_SLOTS:

private Q_SLOTS:
    void onEntitiesSearchFinished(Tpl::PendingOperation*);

private:
    QList<EntityModelItem> m_entities;

};

Q_DECLARE_METATYPE(Tpl::EntityPtr);
Q_DECLARE_METATYPE(Tp::AccountPtr);


#endif // ENTITYMODEL_H
