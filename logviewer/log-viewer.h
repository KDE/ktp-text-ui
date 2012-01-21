#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>
#include <TelepathyQt/AccountManager>

namespace Ui {
    class LogViewer;
}

class EntityModel;

class LogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = 0);
    ~LogViewer();
private Q_SLOTS:
    void onAccountManagerReady();

    void onEntitySelected(const QModelIndex &index);
    void onDateSelected();

private:
    Ui::LogViewer *ui;
    Tp::AccountManagerPtr m_accountManager;
    EntityModel *m_entityModel;
};

#endif // LOGVIEWER_H
