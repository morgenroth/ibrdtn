package de.tubs.ibr.dtn.dtalkie.service;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import de.tubs.ibr.dtn.api.EID;
import de.tubs.ibr.dtn.api.GroupEndpoint;

public class RecorderService extends Service {

	private static final String TAG = "RecorderService";

    public static final GroupEndpoint TALKIE_GROUP_EID = new GroupEndpoint("dtn://dtalkie.dtn/broadcast");
    
    public static final String ACTION_START_RECORDING = "de.tubs.ibr.dtn.dtalkie.START_RECORDING";
    public static final String ACTION_STOP_RECORDING = "de.tubs.ibr.dtn.dtalkie.STOP_RECORDING";
    public static final String ACTION_ABORT_RECORDING = "de.tubs.ibr.dtn.dtalkie.ABORT_RECORDING";
    
    public static final String EVENT_RECORDING_EVENT = "de.tubs.ibr.dtn.dtalkie.RECORDING_EVENT";
    public static final String EVENT_RECORDING_INDICATOR = "de.tubs.ibr.dtn.dtalkie.RECORDING_INDICATOR";
    
    public static final String EXTRA_LEVEL_NORMALIZED = "level_normalized";
    public static final String EXTRA_LEVEL_VALUE = "level_value";
    public static final String EXTRA_LEVEL_MAX = "level_max";
    
    public static final String EXTRA_DESTINATION_ENDPOINT = "destination_endpoint";
    public static final String EXTRA_DESTINATION_ENDPOINT_IS_SINGLETON = "destination_endpoint_singleton";
    public static final String EXTRA_INDICATOR = "indicator";
    public static final String EXTRA_AUTO_STOP = "auto_stop";
    public static final String EXTRA_RECORDING_ACTION = "action";

    private MediaRecorder mRecorder = null;
    private SoundFXManager mSoundManager = null;
    private File mCurrentFile = null;
    
    private Object mRecLock = new Object();
    private Boolean mRecording = false;
    private Boolean mOnEar = false;
    private Boolean mAbort = false;

    private EID mDestination = null;
    
    private boolean mAutoStop = false;
    private boolean mUpdateIndicator = false;
    private int mMaxAmplitude = 0;
    private int mIdleCounter = 0;
    
    private Handler mHandler = null;
    
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    
    /**
     * static methods for better control of recorder service
     */
    
    public static void startRecording(Context context, EID endpoint, boolean indicator, boolean auto_stop) {
        Intent i = new Intent(context, RecorderService.class);
        i.setAction(RecorderService.ACTION_START_RECORDING);
        Utils.putEndpoint(i, RecorderService.TALKIE_GROUP_EID, RecorderService.EXTRA_DESTINATION_ENDPOINT, RecorderService.EXTRA_DESTINATION_ENDPOINT_IS_SINGLETON);
        i.putExtra(RecorderService.EXTRA_INDICATOR, indicator);
        i.putExtra(RecorderService.EXTRA_AUTO_STOP, auto_stop);
        context.startService(i);
    }
    
    public static void stopRecording(Context context) {
        Intent i = new Intent(context, RecorderService.class);
        i.setAction(RecorderService.ACTION_STOP_RECORDING);
        context.startService(i);
    }
    
    public static void abortRecording(Context context) {
        Intent i = new Intent(context, RecorderService.class);
        i.setAction(RecorderService.ACTION_ABORT_RECORDING);
        context.startService(i);
    }
    
    /**
     * end of static methods
     */

    @Override
    public IBinder onBind(Intent intent) {
        return null;
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
            if (!mRecording) stopSelf(msg.arg1);
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
        
        HandlerThread thread = new HandlerThread("HeadsetService");
        thread.start();
        
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
        
        SensorManager sm = (SensorManager) getSystemService(SENSOR_SERVICE);
        List<Sensor> sensors = sm.getSensorList(Sensor.TYPE_PROXIMITY);
        if (sensors.size() > 0) {
            Sensor s = sensors.get(0);
            sm.registerListener(mSensorListener, s, SensorManager.SENSOR_DELAY_NORMAL);
        }
        
        // handler for indicator level updates
        mHandler = new Handler();
        
        // create a media recorder
        mRecorder = new MediaRecorder();
        
        // set recorder listener
        mRecorder.setOnErrorListener(mErrorListener);
        mRecorder.setOnInfoListener(mInfoListener);
        
        // init sound pool
        mSoundManager = new SoundFXManager();
        
        mSoundManager.initialize(AudioManager.STREAM_VOICE_CALL, 4);
        mSoundManager.initialize(AudioManager.STREAM_MUSIC, 4);
        
        mSoundManager.load(this, Sound.BEEP);
        mSoundManager.load(this, Sound.QUIT);
        mSoundManager.load(this, Sound.SQUELSH_LONG);
        mSoundManager.load(this, Sound.SQUELSH_SHORT);
        
        mRecording = false;
        mOnEar = false;
        mAbort = false;
    }

    @Override
    public void onDestroy() {
    	// stop ongoing recording on destroy
        stopRecording();
        
        SensorManager sm = (SensorManager) getSystemService(SENSOR_SERVICE);
        sm.unregisterListener(mSensorListener);
        
        // release all recoder resources
        mRecorder.release();
        
        mHandler = null;
        
        mServiceLooper.quit();
        
        // call super method
        super.onDestroy();
    }

    private void startRecording(EID destination, boolean indicator, boolean auto_stop)
    {
    	synchronized(mRecLock) {
    		if (mRecording) return;
    		
            // set recording parameters
            mDestination = destination;
            mAutoStop = auto_stop;
            mUpdateIndicator = indicator;
            
            Log.i(TAG, "start recording audio for " + mDestination.toString());
            
            File path = Utils.getStoragePath();
            
            if (path == null) {
                Log.e(TAG, "no storage path available");
            }

            try {
                Log.i(TAG, "create temporary file in " + path.getAbsolutePath());

                // create temporary file
                mCurrentFile = File.createTempFile("record", ".3gp", path);
                
                mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
                mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
                mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_WB);
                mRecorder.setAudioSamplingRate(16000);
                mRecorder.setAudioChannels(1);
                mRecorder.setAudioEncodingBitRate(23850);
	            mRecorder.setOutputFile(mCurrentFile.getAbsolutePath());
	            mRecorder.prepare();
	            mRecorder.start();
	            
	            // set state to recording
	            mRecording = true;
	            mRecLock.notifyAll();
	            
                eventRecordingStarted();
            } catch (IOException e) {
                Log.e(TAG, "can not start recording", e);
                eventRecordingFailed();
            } catch (java.lang.RuntimeException e) {
                Log.e(TAG, "can not start recording", e);
                eventRecordingFailed();
            }
    	}
    }
    
    private void stopRecording()
    {
    	synchronized(mRecLock) {
	        // only stop recording in state RECORDING
	        if (!mRecording) return;
	
	        Log.i(TAG, "stop recording audio");
	        
	        try {
	            // stop the recorder
	            mRecorder.stop();
	        } catch (java.lang.IllegalStateException e) {
	            // error - not recording
	        }
	        
	        // wait until mRecording is false
	        while (mRecording) {
				try {
					mRecLock.wait();
				} catch (InterruptedException e) {
					// interrupted
				}
	        }
    	}
    }
    
    private void abortRecording()
    {
        synchronized(mRecLock) {
            // only stop recording in state RECORDING
            if (!mRecording) return;
    
            Log.i(TAG, "stop recording audio");
            
            // stop the recorder
            mRecorder.stop();
            
            // set recording to false
            mAbort = true;
            
	        // wait until mRecording is false
	        while (mRecording) {
				try {
					mRecLock.wait();
				} catch (InterruptedException e) {
					// interrupted
				}
	        }
        }
    }

    private void finalizeRecording(File f)
    {
    	synchronized(mRecLock) {
	        // reset recorder
	        mRecorder.reset();
	
	        if (mAbort) {
	        	// signal aborted recording
	        	eventRecordingFailed();
	            
	            // delete the file
	            f.delete();
	        } else {
	            // signal stopped recording
	        	eventRecordingStopped();
	            
	            // send recorded intent
	            Intent recorded_i = new Intent(this, TalkieService.class);
	            recorded_i.setAction(TalkieService.ACTION_RECORDED);
	            recorded_i.putExtra("recfile", f);
	            recorded_i.putExtra("destination", (Serializable)mDestination);
	            startService(recorded_i);
	        }
	        
            // set recording to false
            mRecording = false;
            mRecLock.notifyAll();
            mAbort = false;
    	}
    }
    
    private void playSound(Sound s) {
        if (mOnEar) {
            mSoundManager.play(this, AudioManager.STREAM_VOICE_CALL, s);
        } else {
            mSoundManager.play(this, AudioManager.STREAM_MUSIC, s);
        }
    }
    
    private SensorEventListener mSensorListener = new SensorEventListener() {
        public void onSensorChanged(SensorEvent event) {
            if (event.values.length > 0) {
                float current = event.values[0];
                float maxRange = event.sensor.getMaximumRange();
                boolean far = (current == maxRange);
                mOnEar = !far;
            }
        }

        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
    };
    
    private MediaRecorder.OnErrorListener mErrorListener = new MediaRecorder.OnErrorListener() {

        @Override
        public void onError(MediaRecorder mr, int what, int extra) {
            Log.d(TAG, "MediaRecorder-Error: " + String.valueOf(what) + " " + String.valueOf(extra));
            
            // abort the recording
            mAbort = true;
            finalizeRecording(mCurrentFile);
            mCurrentFile = null;
        }
        
    };
    
    private MediaRecorder.OnInfoListener mInfoListener = new MediaRecorder.OnInfoListener() {

        @Override
        public void onInfo(MediaRecorder mr, int what, int extra) {
            Log.d(TAG, "MediaRecorder-Info: " + String.valueOf(what) + " " + String.valueOf(extra));
            
            // stop the recording
            finalizeRecording(mCurrentFile);
            mCurrentFile = null;
        }
        
    };

	protected void onHandleIntent(Intent intent, int startId) {
		String action = intent.getAction();
		if (ACTION_START_RECORDING.equals(action)) {
			// read indicator update flag
			boolean indicator = intent.getBooleanExtra(EXTRA_INDICATOR, false);
			
			// read auto-stop flag
			boolean auto_stop = intent.getBooleanExtra(EXTRA_AUTO_STOP, true);
			
			// get destination EID
			EID destination = Utils.getEndpoint(intent.getExtras(), EXTRA_DESTINATION_ENDPOINT, EXTRA_DESTINATION_ENDPOINT_IS_SINGLETON, TALKIE_GROUP_EID);
			
			// start recording
			startRecording(destination, indicator, auto_stop);
		}
		else if (ACTION_STOP_RECORDING.equals(action)) {
			stopRecording();
		}
		else if (ACTION_ABORT_RECORDING.equals(action)) {
			abortRecording();
		}
	}
	
	private void eventRecordingFailed() {
		// make a noise
        playSound(Sound.SQUELSH_SHORT);
        
        Intent i = new Intent(EVENT_RECORDING_EVENT);
        i.putExtra(EXTRA_RECORDING_ACTION, ACTION_ABORT_RECORDING);
        sendBroadcast(i);
	}
	
	private void eventRecordingStarted() {
        // make a noise
        playSound(Sound.BEEP);
        
        Intent i = new Intent(EVENT_RECORDING_EVENT);
        i.putExtra(EXTRA_RECORDING_ACTION, ACTION_START_RECORDING);
        sendBroadcast(i);

        // start amplitude update
        mMaxAmplitude = 0;
        mIdleCounter = 0;
        
        // start indicator updates 
        if (mUpdateIndicator || mAutoStop) {
        	mHandler.postDelayed(mUpdateAmplitude, 100);
        }
	}
	
	private void eventRecordingStopped() {
		// make a noise
		playSound(Sound.QUIT);
		
        // stop amplitude update
        mHandler.removeCallbacks(mUpdateAmplitude);
        
        Intent i = new Intent(EVENT_RECORDING_EVENT);
        i.putExtra(EXTRA_RECORDING_ACTION, ACTION_STOP_RECORDING);
        sendBroadcast(i);
	}
	
    private Runnable mUpdateAmplitude = new Runnable() {
        @Override
        public void run() {
            int curamp = 0;
        	synchronized(mRecLock) {
        		curamp = mRecorder.getMaxAmplitude();
        	}
        	
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
            
            if (mUpdateIndicator) {
	            Intent i = new Intent(EVENT_RECORDING_INDICATOR);
	            i.putExtra(EXTRA_LEVEL_MAX, mMaxAmplitude);
	            i.putExtra(EXTRA_LEVEL_VALUE, curamp);
	            i.putExtra(EXTRA_LEVEL_NORMALIZED, level);
	            sendBroadcast(i);
            }
            
            // if there is no sound for 2 seconds
            if (mAutoStop && (mIdleCounter >= 20)) {
                RecorderService.stopRecording(RecorderService.this);
                return;
            }
            
            // schedule next amplitude update
            mHandler.postDelayed(mUpdateAmplitude, 100);
        }
    };
}
