#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QThread>
#include <QImage>

extern int w;
extern int h;
extern int stopped;
extern int stoppedd;
//extern QImage img;
extern QImage palimg;
using namespace std;

enum {
    FLV_TAG_TYPE_AUDIO = 0x08,
    FLV_TAG_TYPE_VIDEO = 0x09,
    FLV_TAG_TYPE_META  = 0x12,
};

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
class mythread : public QThread
{
    Q_OBJECT
public:
    explicit mythread(QObject *parent = 0);
    void stop();
    bool LabSizeSet;
    bool GetExtradata;
    bool OpenOutFile;
    bool SaveFile;      //保存视频文件标志，新建文件，写视频头
    bool StopSave;      //写视频到文件标志
    bool write_trailer; //写视频文件尾标志
    bool change;
    bool setencoder;
    bool cappic;

    int set_ip_port(QString ip, QString port, int num);
    int set_port(QString str);
    int changsize(int w, int h);
    int capture(int num);
    int auto_save(int au);


signals:
    void getImg(QImage img);
    void set_w_h(int w, int h);
    void timeout();
    void timeout_start();
    void restart();
    void th_quit();

public slots:

private:
//    volatile bool stopped;

    int NUM;
    char *IP;
    char *PORT;
    int out_width;
    int out_height;
    const char *out_path;
    QString str;

    AVFrame *pFrame;
    FILE *OUTFILE;
    char *name;

    int GaveUpFrame;

    int mydst;
    int mypst;

    QImage img;
protected:
    void run();

};

#endif // THREAD_H
