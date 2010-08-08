#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>

namespace Ui
{
class ChatWindowConfig;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

public slots:
    void debugStyleList();

private:
    Ui::ChatWindowConfig *ui;

private slots:
    void sendDemoMessages();
};

#endif // MAINWINDOW_H
