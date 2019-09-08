package com.example.musicplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import com.example.listener.OnParparedListener;
import com.example.log.MyLog;
import com.example.player.WlPlayer;

public class MainActivity extends AppCompatActivity {
    private WlPlayer wlPlayer;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        wlPlayer = new WlPlayer();
        /**
         * 2.解码完毕后，自动播放
         */
        wlPlayer.setWlOnParparedListener(new OnParparedListener() {
            @Override
            public void onParpared() {
                MyLog.d("准备好了，C++层解码完毕，回调到java层，可以开始播放声音了");
                wlPlayer.start();
            }
        });
    }

    /**
     * 1.开始解码，按钮点击出发
     * @param view
     */
    public void begin(View view) {

        wlPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        wlPlayer.parpared();

    }

    /**
     * 播放本地pcm数据
     * @param view
     */
    public void playPcm(View view) {
        String path = "/mnt/sdcard/mydream.pcm";
        wlPlayer.playPcm(path);

    }
}
