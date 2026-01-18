#include "packetqueue.h"

PacketQueue::PacketQueue(): m_mutex(new QMutex)
    , m_cond(new QWaitCondition) {

}

int PacketQueue::packetPut(AVPacket *pkt)
{
    m_mutex->lock();
    pkts.push_back(*pkt);
    size += pkt->size;
    m_cond->notify_all();
    m_mutex->unlock();
    return 0;
}

int PacketQueue::packetGet(AVPacket *pkt, std::atomic<bool> &quit)
{
    int ret = 0;

    m_mutex->lock();

    for(;;) {
        if (!pkts.empty()) {
            AVPacket &firstPkt = pkts.front();

            size -= firstPkt.size;
            *pkt = firstPkt;

            // remove this packet
            pkts.erase(pkts.begin());

            ret = 1;
            break;
        } else {
            //std::unique_lock<std::mutex> lck(*m_mutex); //加锁互斥量
            m_cond->wait(m_mutex,500);
        }

        if (quit) {
            ret = -1;
            break;
        }
    }

    m_mutex->unlock();
    return ret;
}

void PacketQueue::packetFlush()
{
    m_mutex->lock();

    std::list<AVPacket>::iterator iter;
    for (iter = pkts.begin(); iter != pkts.end(); ++iter) {
        AVPacket &pkt = *iter;
        av_packet_unref(&pkt);
    }
    pkts.clear();

    size = 0;
    m_mutex->unlock();
}

int PacketQueue::packetSize()
{
    return size;
}
