package de.tubs.ibr.dtn.dtalkie.service;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.NotificationCompat;
import de.tubs.ibr.dtn.dtalkie.R;
import de.tubs.ibr.dtn.dtalkie.TalkieActivity;
import de.tubs.ibr.dtn.dtalkie.db.MessageDatabase.Folder;

public class HeadsetService extends Service {
    
    public static final String ENTER_HEADSET_MODE = "de.tubs.ibr.dtn.dtalkie.ENTER_HEADSET_MODE";
    public static final String LEAVE_HEADSET_MODE = "de.tubs.ibr.dtn.dtalkie.LEAVE_HEADSET_MODE";
    public static final String ACTION_TOGGLE_RECORD = "de.tubs.ibr.dtn.dtalkie.TOGGLE_RECORD";
    public static final String ACTION_START_RECORD = "de.tubs.ibr.dtn.dtalkie.START_RECORD";
    public static final String ACTION_STOP_RECORD = "de.tubs.ibr.dtn.dtalkie.STOP_RECORD";
    public static final String ACTION_PLAY_ALL = "de.tubs.ibr.dtn.dtalkie.PLAY_ALL";
    
    public static Boolean ENABLED = false;
    
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    
    private Boolean mPersistent = false;
    private AudioManager mAudioManager = null;
    private ComponentName mMediaButtonReceiver = null;
    
    private Boolean mRecording = false;
    
    private SoundFXManager mSoundManager = null;
    
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    private AudioManager.OnAudioFocusChangeListener mAfListener = new AudioManager.OnAudioFocusChangeListener() {
		@Override
		public void onAudioFocusChange(int focusChange) {
			if (focusChange == AudioManager.AUDIOFOCUS_LOSS) {
				Intent i = new Intent(HeadsetService.this, HeadsetService.class);
				i.setAction(HeadsetService.LEAVE_HEADSET_MODE);
				HeadsetService.this.startService(i);
			}
		}
	};
    
    protected void onHandleIntent(Intent intent, int startId) {
        String action = intent.getAction();
        
        if (ENTER_HEADSET_MODE.equals(action)) {
            // create initial notification
            Notification n = buildNotification();

            // turn this to a foreground service (kill-proof)
            startForeground(1, n);
            
            // acquire auto-play lock
            ENABLED = true;
            
            // set service mode to persistent
            mPersistent = true;
        }
        else if (LEAVE_HEADSET_MODE.equals(action)) {
            // turn this to a foreground service (kill-proof)
            stopForeground(true);
            
            // remove auto-play lock
            ENABLED = false;
            
            // set service mode to persistent
            mPersistent = false;
        }
        else if (ACTION_TOGGLE_RECORD.equals(action) && mPersistent) {
            // if we're currently recording ...
            if (mRecording) {
                // stop recording
                stopRecording();
            } else {
                // start recording
                startRecording();
            }
        }
        else if (ACTION_START_RECORD.equals(action) && mPersistent) {
            if (!mRecording) {
                // start recording
                startRecording();
            }
        }
        else if (ACTION_STOP_RECORD.equals(action) && mPersistent) {
            if (mRecording) {
                // stop recording
                stopRecording();
            }
        }
        else if (ACTION_PLAY_ALL.equals(action) && mPersistent) {
            // play all unread messages
            Intent play_i = new Intent(this, TalkieService.class);
            play_i.setAction(TalkieService.ACTION_PLAY_ALL);
            play_i.putExtra("folder", Folder.INBOX.toString());
            startService(play_i);
        }
    }
    
    private void startRecording() {
        if (mRecording) return;
        mRecording = true;
        
        // start recording
        RecorderService.startRecording(this, RecorderService.TALKIE_GROUP_EID, false, true);
    }
    
    private void stopRecording() {
        if (!mRecording) return;
        mRecording = false;
        
        // stop recording
        RecorderService.stopRecording(this);
        
        // stop other stuff
        eventRecordingStopped();
    }
    
    private void eventRecordingStopped() {
        // set recording to false
        mRecording = false;
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
        return START_STICKY;
    }
    
    @Override
    public void onCreate() {
        super.onCreate();
        
        // init persist state
        mPersistent = false;
        
        // init bound state
        mRecording = false;
        
        // init sound pool
        mSoundManager = new SoundFXManager(AudioManager.STREAM_VOICE_CALL, 2);
        
        mSoundManager.load(this, Sound.BEEP);
        mSoundManager.load(this, Sound.QUIT);
        mSoundManager.load(this, Sound.SQUELSH_LONG);
        mSoundManager.load(this, Sound.SQUELSH_SHORT);
        
        // get the audio manager
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        
        HandlerThread thread = new HandlerThread("HeadsetService");
        thread.start();
        
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
        
    	IntentFilter filter = new IntentFilter();
    	filter.addAction(RecorderService.EVENT_RECORDING_EVENT);
    	registerReceiver(mRecorderEventReceiver, filter);
    	
    	// create a media button receiver
    	mMediaButtonReceiver = new ComponentName(getPackageName(), MediaButtonReceiver.class.getName());
    	
    	// request audio-focus
    	mAudioManager.requestAudioFocus(mAfListener, AudioManager.STREAM_MUSIC, AudioManager.AUDIOFOCUS_GAIN);
    	
        // listen to media button events
        mAudioManager.registerMediaButtonEventReceiver(mMediaButtonReceiver);
    }

    @Override
    public void onDestroy() {
        // unlisten to media button events
        mAudioManager.unregisterMediaButtonEventReceiver(mMediaButtonReceiver);
        
        // abandon audio-focus
        mAudioManager.abandonAudioFocus(mAfListener);
        
        // unregister from recorder events
        unregisterReceiver(mRecorderEventReceiver);
        
        mServiceLooper.quit();
        
	    // free sound manager resources
	    mSoundManager.release();
    }
    
    private BroadcastReceiver mRecorderEventReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			if (RecorderService.EVENT_RECORDING_EVENT.equals(intent.getAction())) {
				String action = intent.getStringExtra(RecorderService.EXTRA_RECORDING_ACTION);
				
				if (RecorderService.ACTION_START_RECORDING.equals(action)) {
			        // make a noise
					mSoundManager.play(HeadsetService.this, Sound.BEEP);
				}
				else if (RecorderService.ACTION_STOP_RECORDING.equals(action)) {
					eventRecordingStopped();
					
					// make a noise
					mSoundManager.play(HeadsetService.this, Sound.QUIT);
				}
				else if (RecorderService.ACTION_ABORT_RECORDING.equals(action)) {
					eventRecordingStopped();
					
					// make a noise
					mSoundManager.play(HeadsetService.this, Sound.SQUELSH_SHORT);
				}
			}
		}
    	
    };
    
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

        return builder.build();
    }
}
