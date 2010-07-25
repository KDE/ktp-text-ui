#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "realclienthandler.h"
#include "chatconnection.h"


namespace Ui
{
class MainWindow;
}


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(RealClientHandler*, QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

protected slots:
    void handleNewConnection(ChatConnection* connection);


private:
    Ui::MainWindow *ui;
    RealClientHandler* m_clientHandler;
};

#endif // MAINWINDOW_H
