//
// Created by 李金洹 on 2019-08-25.
//

#include "JHAudio.h"

LJH_Audio::LJH_Audio(JHPlayerStatus * jhPlayerStatus1) {
    this->jhPlayerStatus=jhPlayerStatus1;
    jhQueue=new JHQueue(jhPlayerStatus);

}

LJH_Audio::~LJH_Audio() {

}
