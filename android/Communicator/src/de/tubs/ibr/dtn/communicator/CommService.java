package de.tubs.ibr.dtn.communicator;

import java.io.IOException;
import java.util.HashMap;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.media.AudioManager;
import android.media.SoundPool;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionConnection;
import de.tubs.ibr.dtn.streaming.DtnStreamReceiver;
import de.tubs.ibr.dtn.streaming.Frame;
import de.tubs.ibr.dtn.streaming.MediaType;
import de.tubs.ibr.dtn.streaming.StreamId;

public class CommService extends Service {
    
    private static final String TAG = "CommService";
    
    private static final int NOTIFICATION_ID = 1;
    
    public static final GroupEndpoint COMM_STREAM_ENDPOINT = new GroupEndpoint("dtn://broadcast.dtn/communicator");
    
    public static final String COMM_STATE = "de.tubs.ibr.dtn.communicator.STATE";
    public static final String OPEN_COMM_CHANNEL = "de.tubs.ibr.dtn.communicator.OPEN_COMM";
    public static final String CLOSE_COMM_CHANNEL = "de.tubs.ibr.dtn.communicator.CLOSE_COMM";
    
    public static final String OPEN_COMM_TRANSMISSION = "de.tubs.ibr.dtn.communicator.OPEN_TRANS";
    public static final String FRAME_COMM_TRANSMISSION = "de.tubs.ibr.dtn.communicator.FRAME_TRANS";
    public static final String CLOSE_COMM_TRANSMISSION = "de.tubs.ibr.dtn.communicator.CLOSE_TRANS";
    
    // The communication with the DTN service is done using the DTNClient
    private DTNClient mClient = null;
    private Session mSession = null;
    
    private DtnStreamReceiver mStreamEndpoint = null;
    
    private NotificationCompat.Builder mBuilder = null;
    private Notification mNotification = null;
    
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    
    private SpeexTransmitter mTransmitter = null;
    
    private HashMap<StreamId, SpeexReceiver> mReceivers = new HashMap<StreamId, SpeexReceiver>();
    
    private boolean mPlaying = false;
    private boolean mRecording = false;
    
    private SoundPool mSounds = null;
    private int mChirpSound = 0;
    private int mAckSound = 0;
    private int mChanOpen = 0;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    SessionConnection mSessionListener = new SessionConnection() {
        @Override
        public void onSessionConnected(Session session) {
            // session locally available
            mSession = session;
            
            // start foreground service
            CommService.this.startForeground(NOTIFICATION_ID, mNotification);
            
            // create streaming endpoint with own data handler as fallback
            mStreamEndpoint = new DtnStreamReceiver(CommService.this, session, mStreamListener, null);
        }

        @Override
        public void onSessionDisconnected() {
            mStreamEndpoint.release();
            
            CommService.this.stopForeground(true);
            
            mStreamEndpoint = null;
            mSession = null;
        }
    };
    
    DtnStreamReceiver.StreamListener mStreamListener = new DtnStreamReceiver.StreamListener() {
        @Override
        public void onInitial(StreamId id, MediaType type, byte[] data) {
            // start playing
            Intent ti = new Intent(CommService.this, CommService.class);
            ti.setAction(CommService.OPEN_COMM_TRANSMISSION);
            ti.putExtra("stream_id", (Parcelable)id);
            ti.putExtra("data", data);
            ti.putExtra("type", type);
            startService(ti);
        }

        @Override
        public void onFrameReceived(StreamId id, Frame frame) {
            // forward frame
            Intent ti = new Intent(CommService.this, CommService.class);
            ti.setAction(CommService.FRAME_COMM_TRANSMISSION);
            ti.putExtra("stream_id", (Parcelable)id);
            ti.putExtra("frame", (Parcelable)frame);
            startService(ti);
        }

        @Override
        public void onFinish(StreamId id) {
            // stop play service
            Intent ti = new Intent(CommService.this, CommService.class);
            ti.setAction(CommService.CLOSE_COMM_TRANSMISSION);
            ti.putExtra("stream_id", (Parcelable)id);
            startService(ti);
        }
    };
    
    private class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Intent intent = (Intent) msg.obj;
            onHandleIntent(intent, msg.arg1);
            if (!mRecording && !mPlaying) stopSelf(msg.arg1);
        }
    }
    
    private void broadcastState() {
        // send stick intent
        Intent i = new Intent(COMM_STATE);
        i.putExtra("playing", mPlaying);
        i.putExtra("recording", mRecording);
        sendStickyBroadcast(i);
    }
    
    protected void onHandleIntent(Intent intent, int startId) {
        String action = intent.getAction();
        if (OPEN_COMM_CHANNEL.equals(action)) {
            if ((mSession != null) && !mRecording) {
                mTransmitter = new SpeexTransmitter(this, mSession, COMM_STREAM_ENDPOINT);
                
                // play chirp sound
                mSounds.play(mChirpSound, 1.0f, 1.0f, 1, 0, 1.0f);
                
                // wait until the sound is done
                try {
                    Thread.sleep(300);
                } catch (InterruptedException e) {
                    // interrupted
                }
                
                // start recording
                mTransmitter.start();
                
                Log.d(TAG, "channel open");
                
                // set recording mark
                mRecording = true;
                
                // broadcast service state
                broadcastState();
            }
        }
        else if (CLOSE_COMM_CHANNEL.equals(action)) {
            if (mRecording) {
                mTransmitter.close();
                mTransmitter = null;
                
                // play ack sound
                mSounds.play(mAckSound, 1.0f, 1.0f, 1, 0, 1.0f);
                
                Log.d(TAG, "channel closed");
                
                // remove recording mark
                mRecording = false;
                
                // broadcast service state
                broadcastState();
            }
        }
        else if (OPEN_COMM_TRANSMISSION.equals(action)) {
            StreamId id = intent.getParcelableExtra("stream_id");
            byte[] data = intent.getByteArrayExtra("data");
            
            Log.d(TAG, "incoming transmission: " + id);
            
            SpeexReceiver receiver = new SpeexReceiver(data);
            mReceivers.put(id, receiver);
            
            // start audio receiver
            receiver.start();
        }
        else if (FRAME_COMM_TRANSMISSION.equals(action)) {
            StreamId id = intent.getParcelableExtra("stream_id");
            Frame frame = intent.getParcelableExtra("frame");
            
            SpeexReceiver receiver = mReceivers.get(id);
            if (receiver != null) {
                receiver.push(frame);
                
                if (!mPlaying) {
                    // send stick intent
                    Intent i = new Intent(COMM_STATE);
                    i.putExtra("playing", mPlaying);
                    i.putExtra("recording", mRecording);
                    sendStickyBroadcast(i);
                    
                    // play open channel sound
                    mSounds.play(mChanOpen, 0.5f, 0.5f, 1, 0, 1.0f);
                    
                    // set playing mark
                    mPlaying = true;
                    
                    // broadcast service state
                    broadcastState();
                }
            }
        }
        else if (CLOSE_COMM_TRANSMISSION.equals(action)) {
            StreamId id = intent.getParcelableExtra("stream_id");
            
            Log.d(TAG, "incoming transmission closed: " + id);
            
            SpeexReceiver receiver = mReceivers.get(id);
            
            if (receiver != null) {
                receiver.close();
                mReceivers.remove(id);
                
                if (mReceivers.size() == 0) {
                    mPlaying = false;
                    
                    // broadcast service state
                    broadcastState();
                }
            }
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
        
        // prepare service notification
        mBuilder = new NotificationCompat.Builder(this);
        mBuilder.setContentTitle(getString(R.string.app_name));
        mBuilder.setContentText("Say 'ok, Computer...'");
        mBuilder.setSmallIcon(R.drawable.ic_launcher_flat);
        mBuilder.setOngoing(true);
        mBuilder.setWhen(0);
        
        Intent notifyIntent = new Intent(this, MainActivity.class);
        notifyIntent.setAction("android.intent.action.MAIN");
        notifyIntent.addCategory("android.intent.category.LAUNCHER");

        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notifyIntent, 0);
        mBuilder.setContentIntent(contentIntent);
        
        mNotification = mBuilder.build();
        
        // load sound pool
        mSounds = new SoundPool(1, AudioManager.STREAM_SYSTEM, 0);
        try {
            mChirpSound = mSounds.load(getAssets().openFd("chirp.mp3"), 1);
            mAckSound = mSounds.load(getAssets().openFd("ack.mp3"), 1);
            mChanOpen = mSounds.load(getAssets().openFd("channel-open.mp3"), 1);
        } catch (IOException e) {
            Log.e(TAG, "sound loading failed.", e);
        }
        
        HandlerThread thread = new HandlerThread(TAG);
        thread.start();
        
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
        
        // create a new DTN client
        mClient = new DTNClient(mSessionListener);
        
        // create registration with "ping" as endpoint
        // if the EID of this device is "dtn://device" then the
        // address of this app will be "dtn://device/ping"
        Registration registration = new Registration("communicator");
        
        // additionally join a group
        registration.add(COMM_STREAM_ENDPOINT);
        
        try {
            // initialize the connection to the DTN service
            mClient.initialize(this, registration);
            Log.d(TAG, "Connection to DTN service established.");
        } catch (ServiceNotAvailableException e) {
            // The DTN service has not been found
            Log.e(TAG, "DTN service unavailable. Is IBR-DTN installed?", e);
        } catch (SecurityException e) {
            // The service has not been found
            Log.e(TAG, "The app has no permission to access the DTN service. It is important to install the DTN service first and then the app.", e);
        }
        
        // send stick intent
        Intent i = new Intent(COMM_STATE);
        i.putExtra("recording", false);
        sendStickyBroadcast(i);
    }

    @Override
    public void onDestroy() {
        if (mSounds != null) mSounds.release();
        
        if (mTransmitter != null) {
            mTransmitter.close();
            mTransmitter = null;
        }
        
        mClient.terminate();
        super.onDestroy();
    }
}
