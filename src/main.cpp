#include <QApplication>
#include "windows/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return QApplication::exec();
}
