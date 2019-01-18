#include "readthread.h"
#include <QDebug>
#include <sys/time.h>
#include "cpacketlist.h"

CPacketList	m_pktList; //缓存队列
AVFormatContext *pFormatCtx;
int videoindex;
extern int readstart;  //
int stopped;
int stoppedd;
int while_quit;

Readthread::Readthread(QObject *parent) : QThread(parent)
{
    stopped = 0;
}

int Readthread::set_ip_port(QString ip, QString port, int num)
{
//    NUM = num;
    str = "udp://" + ip + ":" + port;
    return 0;
}

void Readthread::run()
{
    int i;

    //输入文件路径
    QByteArray ba = str.toLatin1();
    out_path = ba.data();

    printf("asdgds:%s\n", out_path);
    qDebug("wait for playing %s\n", out_path);

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    qDebug("size %ld\tduration %ld\n", pFormatCtx->probesize, pFormatCtx->max_analyze_duration);
    pFormatCtx->probesize = 20000000;
    pFormatCtx->max_analyze_duration = 200000;

    AVDictionary* options = NULL;
//    av_dict_set(&options, "fflags", "nobuffer", 0);


    //超时时间
    av_dict_set(&options, "timeout", "15000000", 0); // in us

//    AVDictionary* options = NULL;
//    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    if(avformat_open_input(&pFormatCtx, out_path, /*iformat*/0, &options/*0*/) < 0)
    {
        qDebug("Couldn't open input stream.\n");
        stopped = 0;
        readstart = 0;
        while_quit = 0;
        emit readth_quit();
//        emit timeout();
        return;
    }

//    av_dict_free(&options);

    if (int xxg = avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug("Couldn't find stream information %d.\n", xxg);
        return;
    }

    videoindex = 0;

    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
//        AVFormatContext *ofmt_ctx = NULL;
        AVStream *stream = pFormatCtx->streams[i];
//        AVStream *out_stream = avformat_new_stream(ofmt_ctx, stream->codec->codec);

        if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;


//stream->codec->width = 576;
//stream->codec->height = 720;

//AVDictionaryEntry *m = NULL;
//while((m=av_dict_get(stream->metadata,"",m,AV_DICT_IGNORE_SUFFIX))!=NULL)
//{
//    qDebug<<"=====key:"<<m->key<<" ==== value: "<<m->value;
//}


            if(stream->codec->codec_id == AV_CODEC_ID_H264)
            {
                qDebug("codec_id %d: h.264", i);
            }

            int  frame_rate = 0;
            if(stream->r_frame_rate.den > 0)
            {
                frame_rate = stream->r_frame_rate.num/stream->r_frame_rate.den;

            }
            else if(stream->codec->framerate.den > 0)
            {
                frame_rate = stream->codec->framerate.num/stream->codec->framerate.den;

            }
            qDebug("frame rate : %d", frame_rate);
            break;
        }
    }

    pFrame = av_frame_alloc();
    AVPacket *packet;
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));


    int ww = 0;
    int www = 0;
    int nnn = 0;
    qDebug("read线程开始");
    stopped = 0;
    while (1)
    {
        if(stopped)
        {
            qDebug("read线程结束");
            break;
        }


        if(av_read_frame(pFormatCtx, packet) < 0)
        {
            continue;
        }

//        if(nnn != 0)
//        {
//            nnn--;
//            continue;
//        }

//        ww = pFormatCtx->streams[videoindex]->codec->width;
//        qDebug("ww:%d", ww);
//        qDebug("www:%d", www);
//        if(ww != www)
//        {
//            nnn = 5;
//            www = ww;
//            continue;
//        }
//        www = ww;
//        qDebug()<<"~~~~~~~~~width:"<<pFormatCtx->streams[videoindex]->codec->width;
//        qDebug()<<"~~~~~~~~~height:"<<pFormatCtx->streams[videoindex]->codec->height;
        readstart = 1;
        while_quit = 1;
        m_pktList.InputPacket(packet);

        av_free_packet(packet);
    }
    stoppedd = 0;
    readstart = 0;
    while_quit = 0;
    emit readth_quit();
    avformat_close_input(&pFormatCtx);
}
