#include "QtGUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtGUI w;
    w.initializeUI();
    w.show();
    return a.exec();
}
