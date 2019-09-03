//
// Created by 李金洹 on 2019-08-25.
//


#include "JHFFmpeg.h"
#include "AndroidLog.h"


/**
 * 解码线程的回调
 */
void *decodeFFmpeg(void *data) {
    //强转数据
    LJH_FFmpeg *jhfFmpeg = (LJH_FFmpeg *) (data);
    //解码线程 decode
    jhfFmpeg->decodeFFmpegThread();

    //退出线程
    pthread_exit(&jhfFmpeg->decodeThread);
}

/**
 * 构造方法
 * @param jhPlayerCallJava
 * @param url
 */
LJH_FFmpeg::LJH_FFmpeg(LJH_PlayerCallJava *jhPlayerCallJava, const char *url,JHPlayerStatus *jhPlayerStatus) {

    this->jhPlayerCallJava = jhPlayerCallJava;
    this->url = url;
    this->jhPlayerStatus=jhPlayerStatus;
}

/**
 * 析构方法
 */
LJH_FFmpeg::~LJH_FFmpeg() {

}

/**
 * 解码器准备方法，这里正是开始一个解码流程
 * 1.开一个线程
 */
void LJH_FFmpeg::prepared() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

/**
 * 专门用于解码的线程
 */
void LJH_FFmpeg::decodeFFmpegThread() {
    //导入解码库，注册解码器
    av_register_all();
    //初始化网络解码器，因为要解析网络地址的东西，所以需要初始化网络的
    avformat_network_init();
    //初始化上下文
    avFormatContext = avformat_alloc_context();
    //打开资源，Url,如果==0，初始化失败，否则成功
    if (avformat_open_input(&avFormatContext, url, NULL, NULL) != 0) {
        if (LOG_DEBUG) {
            LOGE("打开url资源失败")
        }
        return;
    }
    //成功的话， 从打开的Url资源里面 找到里面的流
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        if (LOG_DEBUG) {
            LOGE("打开url中的流失败")
        }
        return;
    }
    //循环找到里面的流, nb_streams 代表里面的流的个数
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        //再从流里面的属性codecpar 找到流的类型,找到我们的音频类型 AUDIO类型
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {

            if(ljh_audio==NULL){
                ljh_audio=new LJH_Audio(jhPlayerStatus);
                ljh_audio->streamIndex=i;
                //保存解码器属性参数，里面含有id
                ljh_audio->codecParameters=avFormatContext->streams[i]->codecpar;
            }

        }
    }

    //找到解码器,decoder就是解码器， 需要一个解码器的avcodecId，所以上面循环的地方需要保存下id,这样就能得到解码器
    AVCodec *avCodec=avcodec_find_decoder(ljh_audio->codecParameters->codec_id);
            //判断是否获取到了解码器
            if(!avCodec){
                if (LOG_DEBUG) {
                    LOGE("没有从流中找到解码器")
                }
                return;
            }

     //得到解码器后，还需要一个解码器上下文，所以去audio.h头文件中进行声明解码器上下文
     //先给上下文分配一个内存空间
     ljh_audio->avCodecContext=avcodec_alloc_context3(avCodec);
            if(!ljh_audio->avCodecContext){
                if (LOG_DEBUG) {
                    LOGE("不能够为解码器上下文分配内存空间")
                }
                return;
            }

      //走到这里，就已经拿到解码器上下文了，这个时候，需要的就是将解码器中的属性内容，赋值到解码器上下文中
     int  result= avcodec_parameters_to_context(ljh_audio->avCodecContext,ljh_audio->codecParameters);
         if(result<0){
             if (LOG_DEBUG) {
                 LOGE("不能将属性内容填充到 解码器上下文中")
             }
             return;
         }

         //打开解码器
         int openResult=avcodec_open2(ljh_audio->avCodecContext,avCodec,0);
         if(openResult!=0){
             if (LOG_DEBUG) {
                 LOGE("打开解码器失败，也就是打开流失败")
             }
             return;
         }
        //走到这就是解码成功了，最后就是需要回调到java层进行通知解码完毕
        jhPlayerCallJava->onCallPrepared(CHILD_THREAD);
}


void LJH_FFmpeg::start() {

    //从ffmpeg里面获取avpacket_frame,所以需要先判断下audio是否为空
    if(ljh_audio==NULL){
        if (LOG_DEBUG) {
            LOGE("没有找到AUDIO")
        }
        return;
    }

    //读取帧
    int count=0;
    while(1){
        AVPacket  *avPacket=av_packet_alloc();
        //从avFormatContext上下文中读取数据
        if(av_read_frame(avFormatContext,avPacket)==0){
            //得到avpacket,再得到音频流的
            if(avPacket->stream_index==ljh_audio->streamIndex){

                count++;
                if (LOG_DEBUG) {
                    LOGE("成功解码第 %d 帧 数据",count);
                }
                //解码成功后，不释放数据，而是放到缓存队列中去
                ljh_audio->jhQueue->putAvpacket(avPacket);
            }else{
                //解码完毕，释放资源
                //解码失败释放资源数据
            av_packet_free(&avPacket);
//            //释放av
            av_free(avPacket);
//            avPacket=NULL;
            }
            //解码完毕，释放资源
            //解码失败释放资源数据
//            av_packet_free(&avPacket);
//            //释放av
//            av_free(avPacket);


            //解码成功后，不释放数据，而是放到缓存队列中去
//            ljh_audio->jhQueue->putAvpacket(avPacket);

        }else{
            //解码失败释放资源数据
            av_packet_free(&avPacket);
            //释放av
            av_free(avPacket);
            avPacket=NULL;
            break;
        }
    }

    //模拟数据出队，使用这些数据进行播放 的模拟
    while(ljh_audio->jhQueue->getQueueSize()>0){
        //声明内存空间
        AVPacket * avPacket=av_packet_alloc();
        ljh_audio->jhQueue->popAvpacket(avPacket);
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket=NULL;
    }

    if(LOG_DEBUG){
        LOGD("解码完成")
    }
}
