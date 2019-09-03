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


public:
    LJH_Audio(JHPlayerStatus * jhPlayerStatus1);
    ~LJH_Audio();

};


#endif //MUSICPLAYER_JHAUDIO_H
