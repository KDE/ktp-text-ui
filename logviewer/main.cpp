#include <QtGui/QApplication>
#include "log-viewer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LogViewer w;
    w.show();

    return a.exec();
}
