//
// Created by 李金洹 on 2019-09-02.
//

#include "JHQueue.h"

JHQueue::JHQueue(JHPlayerStatus *jhPlayerStatus) {

    this->jhPlayerStatus=jhPlayerStatus;
    //初始化线程锁
    pthread_mutex_init(&mutexPacket,NULL);
    //初始化线程锁条件变量
    pthread_cond_init(&condPacket,NULL);


}

JHQueue::~JHQueue() {
//        //释放线程锁
//        pthread_mutex_destroy(&mutexPacket);
//        //释放条件变量
//        pthread_cond_destroy(&condPacket);
}

/**
// *   /avpacket解码入队缓存
// * @param avPacket
// * @return
// */
int JHQueue::putAvpacket(AVPacket *avPacket) {
    //先加锁
    pthread_mutex_lock(&mutexPacket);
    //解码后的数据入队
    queueAvpacket.push(avPacket);
    if(LOG_DEBUG ){
        LOGD("缓存如入队一个avpackrt到队列中，个数为：%d",queueAvpacket.size());
    }

    //入队完毕，发送消息出去
    pthread_cond_signal(&condPacket);

    //解锁
    pthread_mutex_unlock(&mutexPacket);

    return 0;
}
///**
// * avpacket解码出队缓存
// * @param avPacket
// * @return
// */
int JHQueue::popAvpacket(AVPacket *avPacket) {

    pthread_mutex_lock(&mutexPacket);
    //出队，获取队列缓存中的avpacket数据
    //采用循环获取, 封装一个类来，进行获取。根据状态获取.采用一个全局变量来控制循环，封装到status类中，表示是否退出

    while(jhPlayerStatus!=NULL&&jhPlayerStatus->isExit){
        if(queueAvpacket.size()>0){
            //从缓存队列中取出队首
            AVPacket * avPacketItem=queueAvpacket.front();
            //拷贝
            if(av_packet_ref(avPacket,avPacketItem)==0){
                //成功，弹出
                queueAvpacket.pop();
            }
            //使用完资源进行释放
            av_packet_free(&avPacketItem);
            av_free(avPacketItem);
            avPacketItem=NULL;
            if(LOG_DEBUG){
                LOGD("从缓存队列中取出一个avpacket，还剩下%d个",queueAvpacket.size());
            }
        }else{
            //没有数据，开始等待
            pthread_cond_wait(&condPacket,&mutexPacket);
        }
    }


    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

/**
 * 获取队列大小
 * @return
 */
int JHQueue::getQueueSize() {
    pthread_mutex_lock(&mutexPacket);
    int size=queueAvpacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}


