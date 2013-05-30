package de.tubs.ibr.dtn.dtalkie.service;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.lang.ref.WeakReference;
import java.util.List;

import android.app.Service;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioManager;
import android.media.MediaRecorder;
import android.os.Binder;
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
    
    public static final String ACTION_RECORD = "de.tubs.ibr.dtn.dtalkie.RECORD";
    public static final String ACTION_STOP = "de.tubs.ibr.dtn.dtalkie.STOP";
    
    public static final GroupEndpoint TALKIE_GROUP_EID = new GroupEndpoint("dtn://dtalkie.dtn/broadcast");

    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;

    private MediaRecorder mRecorder = null;
    private SoundFXManager mSoundManager = null;
//    private FileObserver mObserver = null;
    private File mCurrentFile = null;
    
    private Boolean mRecording = false;
    private Boolean mOnEar = false;

    private EID mDestination = null;
    
    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new LocalBinder();
    
    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class LocalBinder extends Binder {
        public RecorderService getService() {
            return RecorderService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
    
    private static final class ServiceHandler extends Handler {
        private final WeakReference<RecorderService> mService; 
        
        public ServiceHandler(Looper looper, RecorderService service) {
            super(looper);
            mService = new WeakReference<RecorderService>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            Intent intent = (Intent) msg.obj;
            if (intent != null) {
                mService.get().onHandleIntent(intent, msg.arg1);
            }
        }
    }
    
    public void onHandleIntent(Intent intent, int startId) {
        String action = intent.getAction();

        if (ACTION_RECORD.equals(action)) {
            mDestination = (EID)intent.getSerializableExtra("destination");
            if (mDestination != null) {
                startRecording();
            }
        } else if (ACTION_STOP.equals(action)) {
            stopRecording();
            stopSelf(startId);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            stopSelf(startId);
        }
        
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);

        return START_STICKY;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        
        /*
         * incoming Intents will be processed by ServiceHandler and queued in
         * HandlerThread
         */
        HandlerThread thread = new HandlerThread("RecorderService_IntentThread");
        thread.start();
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper, this);
        
        SensorManager sm = (SensorManager) getSystemService(SENSOR_SERVICE);
        List<Sensor> sensors = sm.getSensorList(Sensor.TYPE_PROXIMITY);
        if (sensors.size() > 0) {
            Sensor s = sensors.get(0);
            sm.registerListener(mSensorListener, s, SensorManager.SENSOR_DELAY_NORMAL);
        }
        
        // create a media recorder
        mRecorder = new MediaRecorder();
        mRecorder.setOnErrorListener(mErrorListener);
        mRecorder.setOnInfoListener(mInfoListener);
        
        // init sound pool
        mSoundManager = new SoundFXManager();
        
        mSoundManager.initialize(AudioManager.STREAM_VOICE_CALL, 4);
        mSoundManager.initialize(AudioManager.STREAM_MUSIC, 4);
        
        mSoundManager.load(this, Sound.BEEP);
        mSoundManager.load(this, Sound.QUIT);
        mSoundManager.load(this, Sound.SQUELSH_LONG);
    }

    @Override
    public void onDestroy() {
        if (isRecording()) stopRecording();
        
        SensorManager sm = (SensorManager) getSystemService(SENSOR_SERVICE);
        sm.unregisterListener(mSensorListener);
        
        // stop looper that handles incoming intents
        mServiceLooper.quit();
        
        // release all recoder resources
        mRecorder.release();
        
        // call super method
        super.onDestroy();
    }
    
    public Boolean isRecording() {
        return mRecording;
    }

    private void startRecording()
    {
        if (isRecording()) return;
        
        Log.i(TAG, "start recording audio");
        
        File path = Utils.getStoragePath();
        
        if (path == null) {
            Log.e(TAG, "no storage path available");
        }

        try {
            Log.i(TAG, "create temporary file in " + path.getAbsolutePath());

            // create temporary file
            mCurrentFile = File.createTempFile("record", ".3gp", path);
            
            mRecorder.setAudioSource(MediaRecorder.AudioSource.DEFAULT);
            mRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
            mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_WB);
            mRecorder.setOutputFile(mCurrentFile.getAbsolutePath());
            mRecorder.prepare();
            mRecorder.start();
            
            // set state to recording
            mRecording = true;
            
            // make a noise
            playSound(Sound.BEEP);
            
//            mObserver = new FileObserver(tmpfile.getAbsolutePath(), FileObserver.CLOSE_WRITE) {
//                @Override
//                public void onEvent(int arg0, String arg1) {
//                    if (arg0 == FileObserver.CLOSE_WRITE)
//                    {
//                        try {
//                            Log.d(TAG, "observer finished");
//                            finalizeRecording(tmpfile);
//                            stopWatching();
//                            mObserver = null;
//                        } catch (Exception ex) { }
//                    }
//                }
//            };
//            
//            mObserver.startWatching();
        } catch (IOException e) {
            Log.e(TAG, "can not start recording", e);
        }
    }
    
    private void stopRecording()
    {
        // only stop recording in state RECORDING
        if (!isRecording()) return;

        Log.i(TAG, "stop recording audio");
        
        // stop the recorder
        mRecorder.stop();
        
        // set recording to false
        mRecording = false;
    }

    private void finalizeRecording(File f)
    {
        // reset recorder
        mRecorder.reset();

        // set recording to false
        mRecording = false;
        
        playSound(Sound.QUIT);
        
        // send recorded intent
        Intent recorded_i = new Intent(this, TalkieService.class);
        recorded_i.setAction(TalkieService.ACTION_RECORDED);
        recorded_i.putExtra("recfile", f);
        recorded_i.putExtra("destination", (Serializable)mDestination);
        startService(recorded_i);
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
            playSound(Sound.SQUELSH_LONG);
        }
        
    };
    
    private MediaRecorder.OnInfoListener mInfoListener = new MediaRecorder.OnInfoListener() {

        @Override
        public void onInfo(MediaRecorder mr, int what, int extra) {
            Log.d(TAG, "MediaRecorder-Info: " + String.valueOf(what) + " " + String.valueOf(extra));
            finalizeRecording(mCurrentFile);
            mCurrentFile = null;
        }
        
    };
}
