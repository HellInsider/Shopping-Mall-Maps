#include <QApplication>

#include "device\manager.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    manager m;
    m.LoadData(":/map1/floors.svg", ":/map1/xml_from_excel.xml");
    m.Start();
    return a.exec();
}
