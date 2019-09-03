//
// Created by 李金洹 on 2019-08-25.
//

#ifndef MUSICPLAYER_JHFFMPEG_H
#define MUSICPLAYER_JHFFMPEG_H


#include "JHPlayerCallJava.h"
#include "pthread.h"
#include "JHAudio.h"
#include <linux/stddef.h>

/**
 * 因为解码库是C语言写的，
 */
extern "C" {
#include <libavformat/avformat.h>
};

/**
 * 这个类专门用于解码
 */
class LJH_FFmpeg {

public:

    //需要一个调用java的方法，导入callJava的方法导入进来
    //还需要一个Url
    //因为需要在子线程中使用，所以还需要一个线程
    LJH_PlayerCallJava *jhPlayerCallJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    //声明解码库 中的avformatContext 上下文指针，
    AVFormatContext *avFormatContext = NULL;
    //导入自定义的AUDIO 封装类指针对象
    LJH_Audio *ljh_audio = NULL;

    //状态控制器
    JHPlayerStatus *jhPlayerStatus=NULL;


public:
    LJH_FFmpeg(LJH_PlayerCallJava *jhPlayerCallJava, const char *url,JHPlayerStatus *jhPlayerStatus);

    ~LJH_FFmpeg();

    void prepared();


    void decodeFFmpegThread();

    void  start();

};

#endif //MUSICPLAYER_JHFFMPEG_H
