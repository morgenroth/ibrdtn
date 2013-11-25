
package de.tubs.ibr.dtn.communicator;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
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
    
    private Animation mAnimTransmit = null;
    private Animation mAnimPlay = null;
    
    private boolean mRecording = false;
    private boolean mPlaying = false;
    
    private Handler mHandler = null;
    
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
    
    private void updateAnimation() {
        if (mRecording) {
            mTransmissionIndicator.setBackgroundResource(R.drawable.transmission_indicator);
            mTransmissionIndicator.startAnimation(mAnimTransmit);
            mTransmissionIndicator.setVisibility(View.VISIBLE);
        }
        else if (mPlaying) {
            mTransmissionIndicator.setBackgroundResource(R.drawable.transmission_indicator_play);
            mTransmissionIndicator.startAnimation(mAnimPlay);
            mTransmissionIndicator.setVisibility(View.VISIBLE);
        }
    }
    
    private BroadcastReceiver mStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            mRecording = intent.getBooleanExtra("recording", false);
            mPlaying = intent.getBooleanExtra("playing", false);
            updateAnimation();
        }
    };
    
    Animation.AnimationListener mAnimListener = new Animation.AnimationListener() {
        @Override
        public void onAnimationStart(Animation animation) {
        }
        
        @Override
        public void onAnimationRepeat(Animation animation) {
        }
        
        @Override
        public void onAnimationEnd(Animation animation) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mPlaying || mRecording) {
                        updateAnimation();
                    } else {
                        mTransmissionIndicator.setVisibility(View.INVISIBLE);
                        mTransmissionIndicator.clearAnimation();
                    }
                }
            });
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        mHandler = new Handler();
        
        mTransmissionIndicator = (FrameLayout)findViewById(R.id.indicator_transmission);
        mTransmissionIndicator.setVisibility(View.INVISIBLE);
        
        mAnimTransmit = AnimationUtils.loadAnimation(this, R.anim.transmission);
        mAnimTransmit.setAnimationListener(mAnimListener);
        
        mAnimPlay = AnimationUtils.loadAnimation(this, R.anim.playing);
        mAnimPlay.setAnimationListener(mAnimListener);
        
        mToggle = (ImageButton)findViewById(R.id.button_transmission);
        mToggle.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!mRecording) {
                    Log.d(TAG, "start recording");
                    
                    Intent i = new Intent(MainActivity.this, CommService.class);
                    i.setAction(CommService.OPEN_COMM_CHANNEL);
                    startService(i);
                } else {
                    Log.d(TAG, "stop recording");
                    
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
        
        IntentFilter filter = new IntentFilter(CommService.COMM_STATE);
        Intent state = registerReceiver(mStateReceiver, filter);
        
        if (state != null) {
            mStateReceiver.onReceive(this, state);
        }
    }

    @Override
    protected void onStop() {
        unregisterReceiver(mStateReceiver);
        
        unbindService(mServiceConnection);
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        mHandler = null;
        super.onDestroy();
    }
}
