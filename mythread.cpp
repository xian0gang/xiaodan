#include "mythread.h"
#include <QDebug>
#include<sys/time.h>
#include <QTimer>
#include <QDateTime>
#include "cpacketlist.h"

int readstart;
int xgxg;

//extern QImage img;
extern int w;
extern int h;
extern QString tm;
extern int while_quit;

extern CPacketList m_pktList; //缓存队列
extern AVFormatContext *pFormatCtx;
extern int videoindex;
mythread::mythread(QObject *parent) : QThread(parent)
{
    change = true;
    GaveUpFrame = 3;
    cappic = false;
    while_quit = 1;
    readstart = 0;
    xgxg = 1;
    StopSave = true;
    write_trailer = false;
    SaveFile = false;
//    img = new QImage;
}

int mythread::auto_save(int au)
{
    if(au)
    {
        SaveFile = true;
    }
    else
    {
        write_trailer = true;
    }
}

int mythread::capture(int num)
{
    if(num == 1)
    {
        change = true;
        cappic = true;
    }
    else
    {
        change = true;
        cappic = false;
    }
}

int mythread::set_ip_port(QString ip, QString port, int num)
{
    NUM = num;
    str = "udp://" + ip + ":" + port;
    return 0;
}

int mythread::changsize(int w, int h)
{

    out_width = w;
    out_height = h;
    change = true;
}

void mythread::run()
{
    AVFormatContext *ofmt_ctx = NULL;
    AVOutputFormat *ofmt = NULL;
    const char *out_filename;
qDebug("解码线程进入");
    while (1)
    {

        //  等待收流线程开启
        if(readstart == 1)
        {

        }
        else
        {
            sleep(1);
            continue;
        }
        int now_w, ret, got_picture;

        AVCodecContext *pCodecCtx;
        AVCodec *pCodec;


        pCodecCtx = pFormatCtx->streams[videoindex]->codec;
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

        uint8_t *out_buffer;
        uint8_t *pal_out_buffer;
        AVFrame *pFrameRGB;
        AVFrame *pal_pFrameRGB;
        struct SwsContext *img_convert_ctx;
        struct SwsContext *pal_img_convert_ctx;

        pFrameRGB = av_frame_alloc();
        pal_pFrameRGB = av_frame_alloc();

        LabSizeSet = true;

        stoppedd = 0;
        GetExtradata = true;
        int src_w = 0;

        OpenOutFile = true;
        setencoder = true;

        OUTFILE = NULL;

        pFrame = av_frame_alloc();
        AVPacket *packet;
        packet = (AVPacket *) av_malloc(sizeof(AVPacket));

        int nowtime = QDateTime::currentDateTime().toTime_t();
        int min = nowtime;
        int newmin = 0;

        /****************录像 编码 设置***********************************************/
        //    变量
        FILE *fpSave;
            if ((fpSave = fopen("geth264.h264", "ab")) == NULL) //h264保存的文件名
                return;

/*********************************************************************/
        while (1)
        {

            //退出标志
            if(stoppedd)
            {
                qDebug("解码线程退出");
                break;
            }

            //从缓存队列取出一帧
            AVPacketList *pList = NULL;
            pList = m_pktList.GetPacket();
            if(pList)
            {
                packet = &pList->pkt;
            }
            else
            {
                continue;
            }


            //解码器设置
            if(setencoder)
            {
                if(packet->flags == 1)
                {

                    //固定解码 h264
                    pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
                    if (!pCodec)
                    {
                        qDebug("Codec not found\n");
                        break;
                    }

                    //打开解码器
                    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
                    {
                        qDebug("Could not open codec.\n");
                        break;
                    }
                    qDebug("setencoder");
                    setencoder = false;
                }
                else
                {
                    continue;
                }
            }

            //显示尺寸变化
            if(change)
            {

//                qDebug("w`````````````````````````````:%d",out_width );
//                qDebug("h`````````````````````````````:%d",out_height );
                int pic_w = pFormatCtx->streams[videoindex]->codec->width;
                int pic_h = pFormatCtx->streams[videoindex]->codec->height;
//                qDebug("w1`````````````````````````````:%d",pic_w );
//                qDebug("h1`````````````````````````````:%d",pic_h );

                float bili_w = (float)pic_w / (float)out_width;
                float bili_h = (float)pic_h / (float)out_height;
//                qDebug("w2`````````````````````````````:%f",bili_w);
//                qDebug("h2`````````````````````````````:%f",bili_h);

//                暂时不按比例重新计算，会影响画框
//                if(bili_w <= bili_h)
//                {
//                    out_width = pic_w * out_height / pic_h;
//                }
//                else
//                {
//                    out_height = out_width * pic_h / out_height;
//                }

                if(cappic)
                {
                    out_width = 1920;
                    out_height = 1080;
                }
//                qDebug("w`````````````````````````````:%d",out_width );
//                qDebug("h`````````````````````````````:%d",out_height );
                pal_out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_RGB32, out_width, out_height));
                avpicture_fill( (AVPicture *) pal_pFrameRGB, pal_out_buffer, AV_PIX_FMT_RGB32, out_width, out_height);

                out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_RGB32, out_width, out_height));
                avpicture_fill( (AVPicture *) pFrameRGB, out_buffer, AV_PIX_FMT_RGB32, out_width, out_height);

                img_convert_ctx = sws_getContext(pFormatCtx->streams[videoindex]->codec->width/*576*/,
                                                 pFormatCtx->streams[videoindex]->codec->height/*720*/,
                                                 pCodecCtx->pix_fmt,
                                                 out_width,
                                                 out_height,
                                                 AV_PIX_FMT_RGB32,
                                                 SWS_BICUBIC,
                                                 NULL, NULL, NULL);

                pal_img_convert_ctx = sws_getContext(pFormatCtx->streams[videoindex]->codec->width/*576*/,
                                                 pFormatCtx->streams[videoindex]->codec->height/*720*/,
                                                 pCodecCtx->pix_fmt,
                                                 out_width,
                                                 out_height,
                                                 AV_PIX_FMT_RGB32,
                                                 SWS_BICUBIC,
                                                 NULL, NULL, NULL);
                change = false;
                qDebug("```````````````````````````````````");
            }

            now_w = pFormatCtx->streams[videoindex]->codec->width;
            qDebug()<<"wwwwwwwww:"<<now_w;
//            qDebug()<<"hhhhhhhhh:"<<pFormatCtx->streams[0]->codec->height;

            //画框判断分辨率标志
            switch (now_w)
            {
                case 1920:
                    xgxg = 1;
                    break;
                case 720:
                    xgxg = 0;
                    break;
                case 960:
                    xgxg = 2;
                    break;
                default:
                    break;
            }

            //延迟几帧
            if(GaveUpFrame != 0)
            {
                GaveUpFrame--;
                continue;
            }
            if(src_w != now_w)
            {
                GaveUpFrame = 3;
                src_w = now_w;
                continue;
            }
            src_w = now_w;

            if (packet->stream_index == videoindex)
            {
                //解码
                ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
                if (ret < 0)
                {
                    qDebug("Decode Error.\n");
                }

                if (got_picture)
                {

                    if(OUTFILE != NULL)
                    {
                        fwrite(packet->data, 1, packet->size, OUTFILE);
                    }

                    if(now_w != 720)
                    {
    //                    qDebug("1920");
                        sws_scale(img_convert_ctx,
                                  (const uint8_t* const *) pFrame->data,
                                  pFrame->linesize,
                                  0,
                                  pFrame->height,
                                  pFrameRGB->data,
                                  pFrameRGB->linesize);

    //                    frame_cnt++;
                        QImage image((uchar *)pFrameRGB->data[0], out_width, out_height, QImage::Format_RGB32);
                        if(cappic)
                        {
                            img = image.copy(600, 252, 720, 576);
                        }
                        else
                        {
                            img = image;
                        }
                    }
                    else
                    {
//                        qDebug("else");
                        sws_scale(pal_img_convert_ctx,
                                  (const uint8_t* const *) pFrame->data,
                                  pFrame->linesize,
                                  0,
                                  pFrame->height,
                                  pal_pFrameRGB->data,
                                  pal_pFrameRGB->linesize);

                        QImage palimage((uchar *)pal_pFrameRGB->data[0], out_width, out_height, QImage::Format_RGB32);
                        img = palimage;
                    }
                    emit getImg(img);
//                    fwrite(packet->data, 1, packet->size, fpSave);//写数据到文件中
                    /*********************************************/


                    AVStream *in_stream, *out_stream;
                    if(SaveFile)
                    {

                        NUM = 1;
                        QString filetm = "./video/" + QString::number(NUM,10) + "-" + QDateTime::fromTime_t(min).toString("yyyyMMddhhmmss") + ".mp4";
                        QByteArray ba = filetm.toLatin1();
                        out_filename = ba.data();
                        qDebug()<<out_filename;
                        //Output
                        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename); //RTMP
                        if (!ofmt_ctx)
                        {
                            qDebug( "Could not create output context\n");
                            //            ret = AVERROR_UNKNOWN;
                            break;
                        }
                        ofmt = ofmt_ctx->oformat;
                        AVStream *stream = pFormatCtx->streams[videoindex];
                        AVStream *out_streamx = avformat_new_stream(ofmt_ctx, stream->codec->codec);
                        if (!out_streamx)
                        {
                            qDebug( "Failed allocating output stream\n");
                            //            ret = AVERROR_UNKNOWN;
                            return;
                        }

                        //Copy the settings of AVCodecContext
                        //int ret;
                        ret = avcodec_copy_context(out_streamx->codec, stream->codec);
                        if (ret < 0) {
                            qDebug( "Failed to copy context from input to output stream codec context\n");
                            return;
                        }
                        out_streamx->codec->codec_tag = 0;


                        av_dump_format(ofmt_ctx, 0, out_filename, 1);
                        //Open output URL
                        if (!(ofmt->flags & AVFMT_NOFILE))
                        {
                            int ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
                            if (ret < 0) {
                                qDebug( "Could not open output URL '%s'", out_filename);
                                break;
                            }
                        }
                        //Write file header
                        int ret = avformat_write_header(ofmt_ctx, NULL);
                        if (ret < 0)
                        {
                            qDebug( "Error occurred when opening output URL\n");
                            break;
                        }
                        SaveFile = false;
                        StopSave = false;
                        write_trailer = false;
                        mydst = 0;
                        mypst = 0;
                        in_stream  = pFormatCtx->streams[packet->stream_index];
                        out_stream = ofmt_ctx->streams[packet->stream_index];

                    }

                    if(!StopSave)
                    {

                        //Convert PTS/DTS
                        mydst = mydst + 1;
                        mypst = mypst + 1;
//                      out_stream->time_base.den = 25000;

                        packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
//                      packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_ZERO));

                        packet->dts = av_rescale_q_rnd(packet->dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
//                      packet->dts = av_rescale_q_rnd(packet->dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_ZERO));
//                      packet->pts = mypst;
//                      packet->dts = mydst;
//                      out_stream->time_base.den = 12800;

                        packet->duration = av_rescale_q(packet->duration, in_stream->time_base, out_stream->time_base);
//                      packet->pos = -1;

//                        qDebug()<<"StopSave"<<StopSave;

                        av_write_frame(ofmt_ctx, packet);
                        int xxxx = av_interleaved_write_frame(ofmt_ctx, packet);
//                                            qDebug("xxxx:%d" ,xxxx);
                        newmin = QDateTime::currentDateTime().toTime_t();

                        //10分钟保存一次视频
//                        if((newmin/60) - (min/60) >= 10)
//                        qDebug()<<"write_trailer:"<<write_trailer;

                    }

                    if(write_trailer)
                    {
                        int aa = av_write_trailer(ofmt_ctx);
                        qDebug("写文件尾 aa:%d",aa);
                        SaveFile = false;
                        StopSave = true;
                        write_trailer = false;
                    }
                    /**********************************************/
                }


            }

            av_free_packet(packet);
        }
    end:
        fclose(OUTFILE);
        fclose(fpSave);
    out:
        sws_freeContext(img_convert_ctx);

        av_frame_free(&pFrameRGB);
        av_frame_free(&pFrame);
        avcodec_close(pCodecCtx);

        stoppedd = 0;
        stopped = 1;
        readstart = 0;

        emit th_quit();
        qDebug()<<"write_trailer:"<<write_trailer;
        if(write_trailer)
        {
            SaveFile = false;
            StopSave = true;
            write_trailer = false;
            int aa = av_write_trailer(ofmt_ctx);
qDebug("写文件尾 aa:%d",aa);
            /* close output */
            if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
                avio_close(ofmt_ctx->pb);
            avformat_free_context(ofmt_ctx);
        }
        qDebug("解码线程退出");
        break;
    }
}

void mythread::stop()
{
    while_quit = 0;
    stoppedd = 1;
    if(!StopSave)
    {
        write_trailer = true;
    }

}
