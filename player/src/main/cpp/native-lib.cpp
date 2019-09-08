#include <jni.h>
#include <string>
#include "JHFFmpeg.h"
#include "JHPlayerStatus.h"

//这个库是C语言编写的。所以extern C
extern "C" {
#include <libavformat/avformat.h>
}

/**
 * 因为要操作java层了，所以需要创建一个callJava的指针，也就是我们自己C++层封装calljava的类的指针
 * 但是由于CallJava类的构造函数需要一个JavaVM的指针。 所以这里也需要声明出来，并且赋值。
 *
 * 需要javaVM，这个值从哪里来，那么应该是从jni层的系统onLoad（)函数来
 */
_JavaVM *javaVM = NULL;
LJH_PlayerCallJava *jhPlayerCallJava = NULL;

/**
 * 还需要一个Ffmpeg的一个类来专门解码,
 * 所以导入自己的FFmpeg解码类的指针
 */
LJH_FFmpeg *jhfFmpeg = NULL;

/**
 * 状态
 */
JHPlayerStatus *jhPlayerStatus=NULL;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    //检测  ，env注意传引用 &
    if (vm->GetEnv((void **) (&env), JNI_VERSION_1_4) != JNI_OK) {
        //失败直接返回-1
        return result;
    }
    return JNI_VERSION_1_4;
}

/**
 * 1.准备数据，加载解析音频数据
 *    开线程，解码数据
 *    所以需要一个解码器的类FFmpeg
 *    解码是个异步线程，等到解码完成，有个C++层回调
 *    通过C++的回调，接到解码完成后，然后通过JNI层，从C++层，调用java层的 解码完成方法 onCallBackPreparedFromJni()
 *    同时还需要一个C++层的类，用于封装 C++层调用java的方法
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_WlPlayer_n_1parpared(JNIEnv *env, jobject instance, jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if (jhfFmpeg == NULL) {
        if (jhPlayerCallJava == NULL) {
            //参数里面需要的是指针的话，就传入一个类的引用
            jhPlayerCallJava = new LJH_PlayerCallJava(javaVM, env, &instance);
        }
        //初始化一个状态器，后续进行解码循环时控制
        jhPlayerStatus=new JHPlayerStatus();

        //初始化ffmpeg解码类，但是得通过callJava和url来进行初始化
        jhfFmpeg = new LJH_FFmpeg(jhPlayerCallJava, source,jhPlayerStatus);
        //初始化ffmpeg解码类以后，那么它至少还得提供一个prapared方法和 start方法，所以需要在里面新增方法
        jhfFmpeg->prepared();
    }

//    env->ReleaseStringUTFChars(source_, source);
}

/**
 * 2.开始播放音频,内部开始解码帧数据并读取
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_WlPlayer_n_1start(JNIEnv *env, jobject instance) {

    //调用c++层的start方法，start方法主要是调用解码器中的av_packect_frame，来读取数据
    if(jhfFmpeg!=NULL){
        //所以ffmpeg，还需要一个start方法
        jhfFmpeg->start();
    }
}

extern "C"
{
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}

//引擎接口
SLObjectItf   enginObject=NULL;
//引擎对象
SLEngineItf engineEngine=NULL;

//
SLObjectItf  outputMixObject=NULL;
SLEnvironmentalReverbItf  outputMixEnvironmentalReverb=NULL;
SLEnvironmentalReverbSettings  reverbSettings=SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

//player
SLObjectItf   pcmPlayerObject=NULL;
SLPlayItf     pcmPlayerPlay=NULL;

//缓冲
SLAndroidSimpleBufferQueueItf   pcmSimpleBufferQueue=NULL;

//播放时，需要读取文件，和buffer
FILE * pcmFile;
void *buffer;
uint8_t  *out_Buffer;



int getPcmData(void **pcm){
    int size=0;
    while(!feof(pcmFile)){
        //每次读1个字节，从pcmFile中读取，读到outbuffer中,注意不能写反
       size= fread(out_Buffer,1,44100*2*2,pcmFile);
        if(out_Buffer==NULL){
            LOGD("读取文件流信息完毕");
            break;
        }else{
            LOGD("读取中");
        }
        //然后把Outbuffer赋值给pcm,指向pcm的内存地址
        *pcm=out_Buffer;
        break;
    }
    return size;

}

//缓冲队列的回调函数,给后面缓冲队列注册回调的时候使用
void  pcmBufferQueueCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context){
    //读取完pcmfile后，这里就会开始调用

   int size= getPcmData(&buffer);
    if(buffer!=NULL){
        (*pcmSimpleBufferQueue)->Enqueue(pcmSimpleBufferQueue,buffer,size);
    }


}


/**
 * 播放pcm数据
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_example_player_WlPlayer_playPcm(JNIEnv *env, jobject instance, jstring sourcePath_) {
    const char *sourcePath = env->GetStringUTFChars(sourcePath_, 0);
    //读取文件 和流
    pcmFile=fopen(sourcePath,"r");
    if(pcmFile==NULL){
        if(LOG_DEBUG){
            LOGE("读取文件失败")
        }
    return;
    }
    //1秒钟读取的数据
    out_Buffer= (uint8_t *)malloc(41100 * 2 * 2);




    //1.创建引擎接口
    slCreateEngine(&enginObject,0,0,0,0,0);
    //实例化实现接口
    (*enginObject)->Realize(enginObject,SL_BOOLEAN_FALSE);
    //获取到引擎对象,通过enginObject来创建引擎对象 engineEngine
    (*enginObject)->GetInterface(enginObject,SL_IID_ENGINE,&engineEngine);


    //2.创建混音器
    const SLInterfaceID  mids[1]={SL_IID_ENVIRONMENTALREVERB};
    const SLboolean    mreq[1]={false};

    (*engineEngine)->CreateOutputMix(engineEngine,&outputMixObject,1,mids,mreq);
    (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    (*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironmentalReverb);

    //3.环境
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,&reverbSettings);



    //队列， datasource， 参数
//    SLDataLocator_AndroidBufferQueue  android_queue={SL_DATALOCATOR_ANDROIDBUFFERQUEUE,2};
    SLDataLocator_AndroidBufferQueue  android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM, //类型
            2,//2个声道（立体声）
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN, //播放结束标志
    };
    SLDataSource  slDataSource={&android_queue,&pcm};

//    SLDataSink  audioSnk={&outputMixObject,NULL};

    SLDataLocator_OutputMix  outputMix={SL_DATALOCATOR_OUTPUTMIX,outputMixObject};

    SLDataSink  audioSnk={&outputMix,NULL};


    const SLInterfaceID   ids[1]={SL_IID_BUFFERQUEUE};
    const SLboolean   req[1]={SL_BOOLEAN_TRUE};
    //4.创建播放器. 通过引擎来创建播放器
    (*engineEngine)->CreateAudioPlayer(engineEngine,&pcmPlayerObject,&slDataSource,&audioSnk,1,ids,req);
    (*pcmPlayerObject)->Realize(pcmPlayerObject,SL_BOOLEAN_FALSE);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_PLAY,&pcmPlayerPlay);



    //5.设置缓冲和回调接口
    //得到队列的queue
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_BUFFERQUEUE,&pcmSimpleBufferQueue);

    //bufferqueueue注册回调函数,启动后，会循环的进入callback之中,在callback中就可以播放了
    (*pcmSimpleBufferQueue)->RegisterCallback(pcmSimpleBufferQueue,pcmBufferQueueCallback,NULL);

    //设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,SL_PLAYSTATE_PLAYING);

    //把回调和bufferqueue关联起来
    pcmBufferQueueCallback(pcmSimpleBufferQueue,NULL);


    env->ReleaseStringUTFChars(sourcePath_, sourcePath);
}