#include "drawer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    drawer w;
    w.show();

    return a.exec();
}
