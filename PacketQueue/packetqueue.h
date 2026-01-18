#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H
#include <atomic>
#include <list>
#include <QMutex>
#include <QWaitCondition>
//#include <condition_variable>
#include <QAtomicInteger>
#ifdef __cplusplus
extern "C" {
#include <libavcodec/avcodec.h>
}
#endif
class PacketQueue
{
public:
    PacketQueue();

    int packetPut(AVPacket *pkt);

    int packetGet(AVPacket *pkt, std::atomic<bool> &quit);

    void packetFlush();

    int packetSize();
private:
    std::list<AVPacket> pkts;
    //std::atomic<int> size = 0;
    QAtomicInteger<int> size = 0;
    QMutex *m_mutex;
    //std::condition_variable *cond;
    QWaitCondition *m_cond;
};

#endif // PACKETQUEUE_H
