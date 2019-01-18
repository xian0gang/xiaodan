#include "widget.h"
#include <QApplication>
#include <QFile>
#include "appinit.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


//    QFile qss("./qss/mywhite.css");
//    qss.open(QFile::ReadOnly);
//    qApp->setStyleSheet(qss.readAll());
//    qss.close();

    AppInit::Instance()->start();
    Widget w;
    w.show();

    return a.exec();
}
