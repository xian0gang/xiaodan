#ifndef READTHREAD_H
#define READTHREAD_H

#include <QObject>
#include <QThread>
#include "mythread.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavutil/imgutils.h>
}

using namespace std;

class Readthread : public QThread
{
    Q_OBJECT
public:
    explicit Readthread(QObject *parent = 0);
    int set_ip_port(QString ip, QString port, int num);


signals:
    void readth_quit();
    void timeout();

public slots:
protected:
    char *IP;
    char *PORT;
    const char *out_path;
    QString str;

    AVFrame *pFrame;

private:
    void run();

};

#endif // READTHREAD_H
