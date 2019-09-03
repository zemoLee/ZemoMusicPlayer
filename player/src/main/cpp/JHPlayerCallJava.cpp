//
// Created by 李金洹 on 2019-08-19.
//


#include "JHPlayerCallJava.h"
#include "AndroidLog.h"

/**
 * 构造方法的实现,赋值全局变量
 * @param javaVM
 * @param jniEnv
 * @param obj
 */
LJH_PlayerCallJava::LJH_PlayerCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {
        this->javaVM = javaVM;
        this->jniEnv=env;
        this->jobj=*obj;
        this->jobj=env->NewGlobalRef(jobj);

        jclass  jlz=jniEnv->GetObjectClass(jobj);
        if(!jlz){
                if(LOG_DEBUG){
                        LOGD("获取class类型失败");
                }
                return ;
        }else{
                jmid_Prepared=jniEnv->GetMethodID(jlz,"onCallBackPreparedFromJni","()V");
        }

}

/**
 * 析构函数
 */
LJH_PlayerCallJava::~LJH_PlayerCallJava() {

}

/**
 * C++层的方法，先在C++层，通过这个方法，然后再通过JmethodId的方式，回调到java层去
 * @param threadType
 */
void LJH_PlayerCallJava::onCallPrepared(int threadType) {
    //主线程
    if(threadType==MAIN_THREAD){
        //构造方法中，已经获取到了java层回调方法的methodid,所以直接回调
        jniEnv->CallVoidMethod(jobj,jmid_Prepared);

    }
    //子线程
    else if(threadType==CHILD_THREAD){
        //先获取到子线程的jniEnv,需要通过JavaVm
        JNIEnv  *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv,0)!=JNI_OK){//如果没有获取到jniEnv
        if(LOG_DEBUG){
                LOGE(" 获取子线程jniEnv错误")
            return;
        }
        }
        jniEnv->CallVoidMethod(jobj,jmid_Prepared);
        javaVM->DetachCurrentThread(); //detach掉当前线程
    }
}


