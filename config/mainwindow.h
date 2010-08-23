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
    void accept();

private:
    Ui::ChatWindowConfig *ui;

private slots:
    void sendDemoMessages();
    void onStylesLoaded();
    void updateVariantsList();

    void onStyleSelected(const QString&);
    void onVariantSelected(const QString&);
    void onShowHeaderChanged(bool);
};

#endif // MAINWINDOW_H
