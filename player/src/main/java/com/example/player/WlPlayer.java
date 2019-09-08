package com.example.player;

import android.text.TextUtils;

import com.example.listener.OnParparedListener;
import com.example.log.MyLog;

/**
 * 播放器
 */
public class WlPlayer {

    /**
     * 加载各种库
     */
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }

    /**
     * 数据源
     */
    private String source;//数据源
    public WlPlayer() {}

    /**
     * 设置数据源
     * @param source
     */
    public void setSource(String source)
    {
        this.source = source;
    }


    /**
     * 设置回调接口
     * @param wlOnParparedListener
     */
    private OnParparedListener onParparedListener;
    public void setWlOnParparedListener(OnParparedListener wlOnParparedListener)
    {
        this.onParparedListener = wlOnParparedListener;
    }

    /**
     * 准备
     */
    public void parpared()
    {
        if(TextUtils.isEmpty(source))
        {
            MyLog.d("数据源为空");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_parpared(source);
            }
        }).start();

    }

    /**
     * 开始播放
     */
    public void start()
    {
        if(TextUtils.isEmpty(source))
        {
            MyLog.d("source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }


//    /**
//     * c++回调java的方法
//     */
//    public void onCallParpared()
//    {
//        if(wlOnParparedListener != null)
//        {
//            wlOnParparedListener.onParpared();
//        }
//    }

    public native void n_parpared(String source);
    public native void n_start();

    /**
     * 由C++层准备完成后，回调这个方法
     */
    public void onCallBackPreparedFromJni(){
        if(onParparedListener!=null){
            onParparedListener.onParpared();
        }
    }




    public native void  playPcm(String sourcePath);

}
