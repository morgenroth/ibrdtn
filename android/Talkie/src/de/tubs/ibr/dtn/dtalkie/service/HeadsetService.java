package de.tubs.ibr.dtn.dtalkie.service;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.AudioManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import android.view.KeyEvent;
import de.tubs.ibr.dtn.dtalkie.R;
import de.tubs.ibr.dtn.dtalkie.TalkieActivity;

public class HeadsetService extends Service {
    
    private static final String TAG = "HeadsetService";
    public static final String ENTER_HEADSET_MODE = "de.tubs.ibr.dtn.dtalkie.ENTER_HEADSET_MODE";
    public static final String LEAVE_HEADSET_MODE = "de.tubs.ibr.dtn.dtalkie.LEAVE_HEADSET_MODE";
    public static final String MEDIA_BUTTON_PRESSED = "de.tubs.ibr.dtn.dtalkie.MEDIA_BUTTON_PRESSED";
    
    public static Boolean ENABLED = false;
    
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    
    private Boolean mPersistent = false;
    private AudioManager mAudioManager = null;
    
    private RecorderService mRecService = null;
    private Boolean mRecording = false;
    
    private int mMaxAmplitude = 0;
    private int mIdleCounter = 0;
    private Handler mHandler = null;
    
    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            mRecService = ((RecorderService.LocalBinder)service).getService();
        }

        public void onServiceDisconnected(ComponentName name) {
            mRecService = null;
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    public void onHandleIntent(Intent intent, int startId) {
        String action = intent.getAction();
        
        if (ENTER_HEADSET_MODE.equals(action)) {
            // create initial notification
            Notification n = buildNotification();

            // turn this to a foreground service (kill-proof)
            startForeground(1, n);
            
            if (!mPersistent) {
                // bind to recorder service
                bindService(new Intent(this, RecorderService.class), mConnection, Context.BIND_AUTO_CREATE);
                
                // listen to media button events
                ComponentName receiver = new ComponentName(getPackageName(), MediaButtonReceiver.class.getName());
                mAudioManager.registerMediaButtonEventReceiver(receiver);
                
                // acquire auto-play lock
                ENABLED = true;
            }
            
            // set service mode to persistent
            mPersistent = true;
        }
        else if (LEAVE_HEADSET_MODE.equals(action)) {
            // turn this to a foreground service (kill-proof)
            stopForeground(true);
            
            if (mPersistent) {
                // remove auto-play lock
                ENABLED = false;
                
                // unlisten to media button events
                ComponentName receiver = new ComponentName(getPackageName(), MediaButtonReceiver.class.getName());
                mAudioManager.unregisterMediaButtonEventReceiver(receiver);
                
                // unbind from recorder service
                unbindService(mConnection);
            }
            
            // set service mode to persistent
            mPersistent = false;
        }
        else if (MEDIA_BUTTON_PRESSED.equals(action) && mPersistent) {
            KeyEvent event = (KeyEvent)intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT);
            
            if (KeyEvent.KEYCODE_MEDIA_PLAY == event.getKeyCode()) {
                startRecording();
                Log.d(TAG, event.toString());
            }
            else if (KeyEvent.KEYCODE_MEDIA_STOP == event.getKeyCode()) {
                stopRecording();
                Log.d(TAG, event.toString());
            }
        }
    }
    
    private void startRecording() {
        if (mRecording) return;
        mRecording = true;
        
        mMaxAmplitude = 0;
        mIdleCounter = 0;
        
        mHandler.postDelayed(mUpdateIdle, 100);

        if (mRecService != null) {
            mRecService.startRecording(RecorderService.TALKIE_GROUP_EID);
        }
    }
    
    private void stopRecording() {
        if (!mRecording) return;
        mRecording = false;
        
        mHandler.removeCallbacks(mUpdateIdle);
        
        if (mRecService != null) {
            mRecService.stopRecording();
        }
    }
    
    @SuppressLint("HandlerLeak")
    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Intent intent = (Intent) msg.obj;
            onHandleIntent(intent, msg.arg1);
            if (!mPersistent) stopSelf(msg.arg1);
        }
    }

    @Override
    public void onStart(Intent intent, int startId) {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        onStart(intent, startId);
        return START_NOT_STICKY;
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        // init persist state
        mPersistent = false;
        
        // init bound state
        mRecording = false;
        
        mHandler = new Handler();
        
        // get the audio manager
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        
        HandlerThread thread = new HandlerThread("HeadsetService");
        thread.start();
        
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
    }

    @Override
    public void onDestroy() {
        mServiceLooper.quit();
    }
    
    private Notification buildNotification() {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this);

        builder.setContentTitle(getResources().getString(R.string.service_headset_name));
        builder.setContentText(getResources().getString(R.string.service_headset_desc));
        builder.setSmallIcon(R.drawable.ic_action_headset);
        builder.setOngoing(true);
        builder.setOnlyAlertOnce(true);
        builder.setWhen(0);

        Intent notifyIntent = new Intent(this, TalkieActivity.class);
        notifyIntent.setAction("android.intent.action.MAIN");
        notifyIntent.addCategory("android.intent.category.LAUNCHER");

        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notifyIntent, 0);
        builder.setContentIntent(contentIntent);

        return builder.getNotification();
    }
    
    private Runnable mUpdateIdle = new Runnable() {
        @Override
        public void run() {
            int curamp = mRecService.getMaxAmplitude();
            if (mMaxAmplitude < curamp) mMaxAmplitude = curamp;
            
            float level = 0.0f;
            
            if (mMaxAmplitude > 0) {
                level = Float.valueOf(curamp) / (0.8f * Float.valueOf(mMaxAmplitude));
            }
            
            if (level < 0.4f) {
                mIdleCounter++;
            } else {
                mIdleCounter = 0;
            }
            
            // if there is no sound for 2 seconds
            if (mIdleCounter >= 20) {
                stopRecording();
            } else {
                mHandler.postDelayed(mUpdateIdle, 100);
            }
        }
    };
}
