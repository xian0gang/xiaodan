#ifndef SHOWLABEL_H
#define SHOWLABEL_H

#include <QWidget>
#include <QLabel>

#include "mythread.h"
#include "readthread.h"

#include <QKeyEvent>
class ShowLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ShowLabel(QWidget *parent = 0);
    int start(QString ip, QString port, int num);
    int stop(int num);
    int changesize(int w, int h);
    int capture(int num);
    int auto2save(int au);

protected:
     void mousePressEvent(QMouseEvent* event);
     void keyPressEvent(QKeyEvent *event);
     void mouseDoubleClickEvent(QMouseEvent *);
     void mouseMoveEvent(QMouseEvent *event);
     void mouseReleaseEvent(QMouseEvent *event);
//     void paintEvent(QPaintEvent *);

signals:
    void clicked();
    void setlabelsize(int, int);
    void Point(int src_x, int src_y, int dst_x, int dst_y);
    void thread_quit();

public slots:
    int ShowVideo(QImage img);
    int TimeOut();
    int ReStart();
    int SetSize(int w, int h);
    void t_qiut();

private:
    mythread T1;

    Readthread R1;


    int src_x, src_y;
    int dst_x, dst_y;
    int ww;
    int GaveUpFrame;
    int num;
    int cap;
    QPixmap pix;

};

#endif // SHOWLABEL_H
