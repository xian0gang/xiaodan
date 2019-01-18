#include "cpacketlist.h"
#define PKT_QUE_SIZE 4

CPacketList::CPacketList()
{

    m_pHead = NULL;
    m_pLast = NULL;
    m_nItemCnt = 0;
    m_nInputNum = 0;
    m_nOutputNum = 0;
    InitializeCriticalSection(&m_cs);
}


bool CPacketList::InputPacket(AVPacket *pktIn)
{
    BOOL bRet = TRUE;

    EnterCriticalSection(&m_cs);

    if (m_nItemCnt <= PKT_QUE_SIZE)
    {
        AVPacketList *pList = new  AVPacketList;

        av_copy_packet(&pList->pkt, pktIn);
        pList->next = NULL;
        m_nItemCnt++;

        if (NULL == m_pHead)
        {
            m_pHead = pList;
        }

        if (m_pLast)
        {
            m_pLast->next = pList;
        }
        m_pLast = pList;
        m_nInputNum++;
    }
    else
    {
        //FreeAllPacket();
        bRet = FALSE;
    }

    LeaveCriticalSection(&m_cs);


    return bRet;
}
AVPacketList* CPacketList::GetPacket( )
{
    AVPacketList *pPkt = NULL;

    int nPos = 0;

    EnterCriticalSection(&m_cs);

    if (m_nItemCnt&&m_pHead)
    {
        pPkt = m_pHead;

        if (pPkt == m_pLast)
        {
            m_pLast = NULL;
        }
        m_pHead = pPkt->next;
        m_nItemCnt--;
        m_nOutputNum++;
    }


    LeaveCriticalSection(&m_cs);
    return pPkt;
}

void CPacketList::FreeAllPacket()
{
    EnterCriticalSection(&m_cs);
    AVPacketList *pNext = NULL;
    while (m_pHead)
    {
        pNext = m_pHead->next;
        av_free_packet(&m_pHead->pkt);
        delete m_pHead;
        m_pHead = pNext;
    }
    m_pLast = NULL;
    m_nItemCnt = 0;
    LeaveCriticalSection(&m_cs);

}
