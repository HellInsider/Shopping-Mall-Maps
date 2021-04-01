#include <QApplication>

#include "device\mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.LoadFile(":/map1/a.svg", ":/map1/xml_from_excel.xml");
    w.Show();
    return a.exec();
}
