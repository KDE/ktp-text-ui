#ifndef MESSAGEVIEW_H
#define MESSAGEVIEW_H

#include "adium-theme-view.h"

#include <QDate>

#include <TelepathyLoggerQt4/Entity>
#include <TelepathyLoggerQt4/PendingOperation>


class MessageView : public AdiumThemeView
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = 0);

    void loadLog(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity, const QDate &date);

private Q_SLOTS:
   void onLoadFinished();
   void onEventsLoaded(Tpl::PendingOperation* po);

private:
    Tpl::EntityPtr m_entity;
    Tp::AccountPtr m_account;
    QDate m_date;

};

#endif // MESSAGEVIEW_H
