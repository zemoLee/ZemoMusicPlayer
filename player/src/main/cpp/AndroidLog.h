//
// Created by 李金洹 on 2019-08-18.
//

#ifndef MUSICPLAYER_ANDROIDLOG_H
#define MUSICPLAYER_ANDROIDLOG_H

#endif //MUSICPLAYER_ANDROIDLOG_H

#include "android/log.h"
#define LOG_DEBUG true

#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"Player_Log",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"Player_Log",FORMAT,##__VA_ARGS__);