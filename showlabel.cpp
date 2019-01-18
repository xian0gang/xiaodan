#include "showlabel.h"
#include <QDebug>
#include <QImage>
#include <QPainter>
QImage img;
QImage palimg;
int w;
int h;
int xxg;
int nuu;
extern int xgxg;

extern int www;
extern int hhh;
extern  int mode;

ShowLabel::ShowLabel(QWidget *parent) : QLabel(parent)
{
    cap = 0;
    xxg = 0;
    nuu = 0;

    src_x = 0;
    src_y = 0;
    dst_x = 0;
    dst_y = 0;
    this->setStyleSheet("border-width: 1px;border-style: solid;border-color: rgb(0, 0, 0);");

    connect(&T1,SIGNAL(getImg(QImage)),this,SLOT(ShowVideo(QImage)));
    connect(&T1,SIGNAL(timeout()),this,SLOT(TimeOut()));
    connect(&T1,SIGNAL(restart()),this,SLOT(ReStart()));
    connect(&T1,SIGNAL(set_w_h(int,int)),this,SLOT(SetSize(int,int)));
    connect(&T1,SIGNAL(th_quit()),this,SLOT(t_qiut()));
    connect(&R1,SIGNAL(readth_quit()),this,SLOT(t_qiut()));
}

int ShowLabel::auto2save(int au)
{
    qDebug()<<"auto:"<<au;
    T1.auto_save(au);
}

void ShowLabel::t_qiut()
{
    emit thread_quit();
}

int ShowLabel::SetSize(int w,int h)
{
    emit setlabelsize(w, h);
}

int ShowLabel::TimeOut()
{
    this->setText("超时，请重试......");
}

//void ShowLabel::paintEvent(QPaintEvent *)
//{
//    QPainter painter(this);

//    // 反走样
//    painter.setRenderHint(QPainter::Antialiasing, true);

//    if(nuu != 0)
//    {
//        nuu--;
//        return;
//    }
//    if(xgxg != xxg)
//    {
//        xxg = xgxg;
//        nuu = 5;
//        return;
//    }
//    xxg = xgxg;


////    QRect rect(0,0,this->width(),this->height());


////    if(pix.isNull())
////    {
////        painter.setPen(QColor(0, 160, 230));
////        painter.drawText(rect(), QStringLiteral("udp://192.168.1.1"));
////    }
////    else
//    {
//        // 绘制图标
//        painter.drawPixmap(rect(), pix);

//        painter.setPen(QPen(Qt::blue,4));//设置画笔形式
//        if(src_x != 0)
//        {
//            if(num != 0)
//            {
//                painter.drawRect(src_x,src_y,dst_x - src_x,dst_y - src_y);//画矩形
//                num--;
//            }
//        }
//    }
//}

void ShowLabel::mousePressEvent(QMouseEvent* event)
{

    if(event->button() == Qt::LeftButton)
    {
        qDebug("clicked");
        emit clicked();
    }
    if(event->button() == Qt::RightButton)
    {
        src_x = event->x();
        src_y = event->y();
    }
}

void ShowLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        int x1,x2,y1,y2;
        if(xgxg == 1)
        {
             x1 = src_x * 1920 / this->width();
             y1 = src_y * 1080 / this->height();
             x2 = dst_x * 1920 / this->width();
             y2 = dst_y * 1080 / this->height();
        }
        else
        {
             x1 = src_x * 720 / this->width();
             y1 = src_y * 576 / this->height();
             x2 = dst_x * 720 / this->width();
             y2 = dst_y * 576 / this->height();
        }

        qDebug("x1:%d y1:%d x2:%d y2:%d", x1,y1,x2,y2);
        emit Point(x1, y1, x2, y2);
    }
}

void ShowLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::RightButton)
    {

        dst_x = event->x();
        dst_y = event->y();
        num = 8;

        update();
    }
}

int ShowLabel::ReStart()
{
    T1.start();
}

int ShowLabel::stop(int num)
{
    switch (num) {
    case 1:
        qDebug("stop one");
        if(T1.isRunning())
        {
            T1.stop();
//            T1.wait();
//            T1.exit();
//            R1.stop();
//            R1.wait();
//            R1.exit();
        }
        break;

    default:
        break;
    }
}

int ShowLabel::start(QString ip, QString port, int num)
{

    QString xg = ip + ":" + port;

    this->setText("waiting: " + xg + "......");
    switch (num) {
    case 1:
        qDebug("thread one");
//        T1.set_ip_port(ip, port, num);
        if(!(T1.isRunning()))
        {
            T1.start();
        }
        R1.set_ip_port(ip, port, num);
        R1.start();
        break;

    default:
        break;
    }

}

int ShowLabel::ShowVideo(QImage imgxg)
{

    if(ww != this->width())
    {
        GaveUpFrame = 3;
        ww = this->width();
        return 0;
    }
    if(GaveUpFrame != 0)
    {
        GaveUpFrame--;
        return 0;
    }
    ww = this->width();

    pix.convertFromImage(imgxg);

//    update();



this->setPixmap(pix);
//    //居中显示
    this->setAlignment(Qt::AlignCenter);
//    this->setPixmap(QPixmap::fromImage(imgxg));
}

//改变
int ShowLabel::changesize(int w, int h)
{
//    if(T1.isRunning())
//    {
        T1.changsize(w, h);
//    }
}

//双击全屏
void ShowLabel::mouseDoubleClickEvent(QMouseEvent *)
{
//    T.start();
    int w, h;
    if(this->isFullScreen())
    {
        this->setWindowFlags(Qt::SubWindow);
        if(mode == 2)
        {
            qDebug()<<"mode:"<<mode;

            qDebug()<<"resize www:"<<www;
            this->resize(www, hhh);
            changesize(www, hhh);
            this->clear();
            //此处必须设置
            this->showNormal();
        }
        else
        {
            //此处必须设置
            this->showNormal();
            //        此处必须设置，否则会有卡顿
            this->clear();
            changesize(this->width(), this->height());
        }
    }else{
        this->setWindowFlags(Qt::Window);
        this->showFullScreen();
        this->clear();

        w = this->width();
        h = this->height();
        changesize(w, h);
    }
//    return true;

}

//esc 取消全屏
void ShowLabel::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        if(this->isFullScreen())
        {
            this->setWindowFlags(Qt::SubWindow);
            this->showNormal();
            this->clear();

            int w, h;
            w = this->width();
            h = this->height();
            changesize(w, h);
        }
    }
}


int ShowLabel::capture(int num)
{
    cap = num;
    T1.capture(num);
}
