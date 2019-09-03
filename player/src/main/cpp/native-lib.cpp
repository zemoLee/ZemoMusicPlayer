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