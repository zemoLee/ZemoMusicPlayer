//
// Created by 李金洹 on 2019-09-02.
//

#ifndef MUSICPLAYER_JHQUEUE_H
#define MUSICPLAYER_JHQUEUE_H


#include "queue"
#include "pthread.h"

extern "C" {
#include "libavcodec/avcodec.h"
};


#include "AndroidLog.h"
#include "JHPlayerStatus.h"
/**
 * 缓存avpacket
 */
class JHQueue {
public:
    //队列
    std::queue<AVPacket *> queueAvpacket;
    //线程锁
    pthread_mutex_t mutexPacket;
    //锁变量
    pthread_cond_t condPacket;

    JHPlayerStatus *jhPlayerStatus=NULL;
public :
     JHQueue(JHPlayerStatus *jhPlayerStatus);

    ~JHQueue();

    //avpacket解码入队缓存
    int putAvpacket(AVPacket *avPacket);

    //avpacket解码出队缓存
    int popAvpacket(AVPacket *avPacket);

    int getQueueSize();


};


#endif //MUSICPLAYER_JHQUEUE_H
