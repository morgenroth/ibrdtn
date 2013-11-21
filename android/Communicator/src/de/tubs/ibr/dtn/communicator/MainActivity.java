
package de.tubs.ibr.dtn.communicator;

import java.io.IOException;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.FrameLayout;
import android.widget.ImageButton;

public class MainActivity extends Activity {
    
    private static final String TAG = "MainActivity";
    private ImageButton mToggle = null;
    private FrameLayout mTransmissionIndicator = null;
    private Animation mTransmissionAnim = null;
    
    private boolean mActivated = false;
    private SoundPool mSounds = null;
    private int mChirpSound = 0;
    
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "service connected");
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "service disconnected");
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        // load sound pool
        mSounds = new SoundPool(1, AudioManager.STREAM_SYSTEM, 0);
        try {
            mChirpSound = mSounds.load(getAssets().openFd("chirp.mp3"), 1);
        } catch (IOException e) {
            Log.e(TAG, "sound loading failed.", e);
        }
        
        mTransmissionIndicator = (FrameLayout)findViewById(R.id.indicator_transmission);
        mTransmissionIndicator.setVisibility(View.INVISIBLE);
        
        mTransmissionAnim = AnimationUtils.loadAnimation(this, R.anim.transmission);
        
        mToggle = (ImageButton)findViewById(R.id.button_transmission);
        mToggle.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!mActivated) {
                    mActivated = true;
                    Log.d(TAG, "start recording");
                    
                    // play chirp sound
                    mSounds.play(mChirpSound, 1.0f, 1.0f, 1, 0, 1.0f);
                    
                    // wait until the sound is done
                    try {
                        Thread.sleep(300);
                    } catch (InterruptedException e) {
                        // interrupted
                    }
                    
                    mTransmissionIndicator.startAnimation(mTransmissionAnim);
                    mTransmissionIndicator.setVisibility(View.VISIBLE);
                    
                    Intent i = new Intent(MainActivity.this, CommService.class);
                    i.setAction(CommService.OPEN_COMM_CHANNEL);
                    startService(i);
                } else {
                    mActivated = false;
                    Log.d(TAG, "stop recording");
                    
                    mTransmissionIndicator.clearAnimation();
                    mTransmissionIndicator.setVisibility(View.INVISIBLE);
                    
                    Intent i = new Intent(MainActivity.this, CommService.class);
                    i.setAction(CommService.CLOSE_COMM_CHANNEL);
                    startService(i);
                }
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    protected void onStart() {
        super.onStart();
        bindService(new Intent(this, CommService.class), mServiceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        unbindService(mServiceConnection);
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        if (mSounds != null) mSounds.release();
        super.onDestroy();
    }
}
