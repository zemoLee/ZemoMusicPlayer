//
// Created by 李金洹 on 2019-08-19.
//


#ifndef MUSICPLAYER_JHPLAYERCALLJAVA_H
#define MUSICPLAYER_JHPLAYERCALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>
#include "AndroidLog.h"

#define  MAIN_THREAD 0
#define  CHILD_THREAD 1

/**
 * 用于C++层准备完成后，封装方法回调java层  准备完成（）方法
 */
class LJH_PlayerCallJava {
public:
    /**
     * 全局的3个变量
     */

    _JavaVM *javaVM = NULL;//用于子线程获取env环境变量
    JNIEnv *jniEnv = NULL;  //用于主线程直接调用
    jobject jobj;

    jmethodID  jmid_Prepared;//C++回调java层的方法的  methodId

public:
    /**
     * 构造
     * @param javaVM
     * @param env
     * @param obj
     */
    LJH_PlayerCallJava(_JavaVM *javaVM ,JNIEnv *env,jobject *obj);
    /**
     * 析构
     */
    ~LJH_PlayerCallJava();

    void onCallPrepared(int threadType);

};

#endif //MUSICPLAYER_JHPLAYERCALLJAVA_H

