//
// Created by 李金洹 on 2019-08-25.
//

#ifndef MUSICPLAYER_JHAUDIO_H
#define MUSICPLAYER_JHAUDIO_H


extern "C"{
//#include "libavcodec/avcodec.h"
#include <libavcodec/avcodec.h>

};

#include "JHQueue.h"
#include "JHPlayerStatus.h"

/**
 * 自定义一个AUDIO类，包装一些东西
 * 1.加一个当前这个流的索引 Index
 */
class LJH_Audio{

public:
    //当前解码出来的流的索引。  自定义的。每解码出一个，进行自增1
    int streamIndex=-1;

    //解码器内部属性对象集合
    AVCodecParameters *codecParameters=NULL;

    //解码器上下文对象
    AVCodecContext  *avCodecContext=NULL;

    //声明队列和状态
    JHQueue * jhQueue=NULL;
    JHPlayerStatus *jhPlayerStatus=NULL;

    //声明一个播放的线程
    pthread_t  thread_play;

    //解码包
    AVPacket *avPacket=NULL;

    //avopacket解码是否成功标志
    int ret=-1;

    //解码接收的 帧
    AVFrame *avFrame=NULL;
    //swr_convert中需要的Buffder, 在构造函数中进行初始化
    uint8_t *buffer=NULL;

    //采样数据大小
    int data_size=0;

public:
    LJH_Audio(JHPlayerStatus * jhPlayerStatus1);
    ~LJH_Audio();

    //重采样，返回大小
    int  resampleAudio();
    //播放，重采样后进行播放
    void play();

};


#endif //MUSICPLAYER_JHAUDIO_H
