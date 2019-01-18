#ifndef COMMONHELPER_H
#define COMMONHELPER_H
#include <QFile>
#include <QApplication>

class CommonHelper
{
public:
    CommonHelper();
    static void setStyle(const QString &style)
    {
            QFile qss(style);
            qss.open(QFile::ReadOnly);
            qApp->setStyleSheet(qss.readAll());
            qss.close();
    }


};

#endif // COMMONHELPER_H
