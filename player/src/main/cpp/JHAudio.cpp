//
// Created by 李金洹 on 2019-08-25.
//


extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

#include "JHAudio.h"

LJH_Audio::LJH_Audio(JHPlayerStatus *jhPlayerStatus1) {
    this->jhPlayerStatus = jhPlayerStatus1;
    jhQueue = new JHQueue(jhPlayerStatus);
    //初始化Buffer
    buffer= (uint8_t *)(av_malloc(44100 * 2 * 2));//1秒采样的大小

}

LJH_Audio::~LJH_Audio() {

}


/**
 * 线程播放的回调函数. 子线程
 * @param data
 * @return
 */
void *decodPlay(void *data) {

    //首先拿到参数数据进行处理
    LJH_Audio *ljh_audio = (LJH_Audio *) (data);


    //解码完成，并且重采样完毕，开始使用数据进行播放，并回调给上层  。 resample->avpacket->进入队列或者阻塞--》所以在解码之前，可以调用下audio
    ljh_audio->resampleAudio();


    pthread_exit(&ljh_audio->thread_play);
}


/**
 * 解码重采样后，需要将pcm数据写入到一个文件中，然后再进行播放
 */
 //TODO
FILE *outFile=fopen("/mnt/sdcard/test_resample2.pcm","w");



/**
 * 重采样后，进行播放。采样是在一个线程，播放是在一个线程，
 */
void LJH_Audio::play() {
    pthread_create(&thread_play, NULL, decodPlay, this);
}

/**
 * 重采样audio资源. 解码avpacket成功后，开启重采样
 */
int LJH_Audio::resampleAudio() {
    //状态不为空，且没有退出
    while (jhPlayerStatus != NULL && !jhPlayerStatus->isExit) {
        //先给avpacket分配一块内存空间
        avPacket = av_packet_alloc();
        //从队列里面，得到avpacket, !=0失败
        if (jhQueue->popAvpacket(avPacket) != 0) {
            if (LOG_DEBUG) {
                LOGE("从解码缓存的队列中，回去队首的avpacket失败")
            }
            //失败，释放资源
            //第一次释放：释放avpacket里面的资源
            //第二次释放，释放avpacket自身的空间
            //内存空间不用了的话，得置空，不置空的话，这个指针还会指向这个内存空间，里面有可能就是另外的值了，有可能导致后面存取值出现问题
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            //失败的话调用下一帧的解码
            continue;
        }
        //成功：
        //将avpacket放到解码器中进行解码,传入解码器的上下文,返回Int值，判断是否成功。 --->>>将avframe发送到解码器
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
        }
        //解码完成后，用avframe来接收,再去头文件声明一个avframe接收解码数据
        //1.先给avframe分配一个内存空间，2.传入解码器上下问, 3.从解码器接收数据      -----<<<<从解码器接收avframe
        ret = avcodec_receive_frame(avCodecContext, avFrame);

        //成功:进行重采样
        if (ret == 0) {
            //设置声道布局
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                //根据声道数，设置声道布局
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                //根据声道布局，设置声道数
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            //重采样上下文  --->swresample 包
            SwrContext *swrContext;
            //给重采样上下文设置参数
            swrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,//输出立体声
                    AV_SAMPLE_FMT_S16,//输出无符号16位
                    avFrame->sample_rate,//输出采样率
                    avFrame->channel_layout,//输入
                    (AVSampleFormat)(avFrame->format),//输入 重采样位数
                    avFrame->sample_rate,//输入采样率
                    NULL,
                    NULL
            );

            //判断重采样上下文设置是否成功,没设置成功是个NULL,
            if(!swrContext||swr_init(swrContext)<0){
                //失败
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if(swrContext!=NULL){
                    swr_free(&swrContext);
                }
                continue;
            }
            //成功。 成功后进行调用,进行转换
            //获取到采样个数
            int nb=swr_convert(
                    swrContext,
                    &buffer,//需要一个Buffer，这里声明到全局去
                   avFrame->nb_samples, //输出的采样个数
                    (const  uint8_t **)(avFrame->data),//输入的数据
                   avFrame->nb_samples //输入的采样个数
                    );
            //根据buffer，获取里面的data size。 根据采样个数，返回实际采样的大小

            //先获取声道数
            int out_channels=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            //采样数据大小算法
            data_size=nb*out_channels*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            //TODO: 将pcm数据写入文件中， 后续进行opensl 播放
            fwrite(buffer,1,data_size,outFile);

            LOGE("重采样数据大小= %d",data_size);
            //释放资源
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            //还要释放avframe
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            swrContext=NULL;
        }
            //失败
        else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            //还要释放avframe
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }

    }
    return 0;
}


