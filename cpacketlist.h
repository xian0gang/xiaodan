#ifndef CPACKETLIST_H
#define CPACKETLIST_H
#include <QObject>
#include <QThread>
#include <windows.h>

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
class CPacketList
{
public:
    CPacketList();

    bool InputPacket(AVPacket *pktIn);
    AVPacketList* GetPacket();
    void FreeAllPacket();

private:
    AVPacketList		*m_pHead;											//列表头
    AVPacketList		*m_pLast;                                           //列表尾
    uint                m_nItemCnt;                                     //共有多少未读项
    uint                m_nInputNum;
    uint                m_nOutputNum;
    CRITICAL_SECTION	m_cs;

};

#endif // CPACKETLIST_H
