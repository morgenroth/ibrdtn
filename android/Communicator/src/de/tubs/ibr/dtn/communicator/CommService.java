package de.tubs.ibr.dtn.communicator;

import java.util.HashMap;

import android.app.Notification;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.DTNClient.Session;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.ServiceNotAvailableException;
import de.tubs.ibr.dtn.api.SessionConnection;
import de.tubs.ibr.dtn.streaming.DtnInputStream;
import de.tubs.ibr.dtn.streaming.Frame;
import de.tubs.ibr.dtn.streaming.MediaType;
import de.tubs.ibr.dtn.streaming.StreamEndpoint;
import de.tubs.ibr.dtn.streaming.StreamId;

public class CommService extends Service {
    
    private static final String TAG = "CommService";
    
    private static final int NOTIFICATION_ID = 1;
    
    public static final GroupEndpoint COMM_STREAM_ENDPOINT = new GroupEndpoint("dtn://broadcast.dtn/communicator");
    
    public static final String OPEN_COMM_CHANNEL = "de.tubs.ibr.dtn.communicator.OPEN_COMM";
    public static final String CLOSE_COMM_CHANNEL = "de.tubs.ibr.dtn.communicator.CLOSE_COMM";
    
    // The communication with the DTN service is done using the DTNClient
    private DTNClient mClient = null;
    private Session mSession = null;
    
    private StreamEndpoint mStreamEndpoint = null;
    
    private NotificationCompat.Builder mBuilder = null;
    private Notification mNotification = null;
    
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    
    private SpeexTransmitter mTransmitter = null;
    
    private HashMap<StreamId, SpeexReceiver> mReceivers = new HashMap<StreamId, SpeexReceiver>();

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
            mStreamEndpoint = new StreamEndpoint(CommService.this, session, mPacketStreamListener, null);
        }

        @Override
        public void onSessionDisconnected() {
            mStreamEndpoint.release();
            
            CommService.this.stopForeground(true);
            
            mStreamEndpoint = null;
            mSession = null;
        }
    };
    
    DtnInputStream.PacketListener mPacketStreamListener = new DtnInputStream.PacketListener() {
        @Override
        public void onInitial(StreamId id, MediaType type, byte[] data) {
            Log.d(TAG, "stream initiated: " + id);
            
            SpeexReceiver receiver = new SpeexReceiver(data);
            mReceivers.put(id, receiver);
            
            receiver.start();
        }

        @Override
        public void onFrameReceived(StreamId id, Frame frame) {
            SpeexReceiver receiver = mReceivers.get(id);
            if (receiver != null) {
                receiver.push(frame);
            }
        }

        @Override
        public void onFinish(StreamId id) {
            Log.d(TAG, "stream closed: " + id);
            
            SpeexReceiver receiver = mReceivers.get(id);
            if (receiver != null) {
                receiver.close();
                mReceivers.remove(id);
            }
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
            stopSelf(msg.arg1);
        }
    }
    
    protected void onHandleIntent(Intent intent, int startId) {
        String action = intent.getAction();
        if (OPEN_COMM_CHANNEL.equals(action)) {
            if (mSession != null) {
                mTransmitter = new SpeexTransmitter(this, mSession, COMM_STREAM_ENDPOINT);
                
                // TODO: make a noise
                mTransmitter.start();
            }
        }
        else if (CLOSE_COMM_CHANNEL.equals(action)) {
            if (mTransmitter != null) {
                mTransmitter.close();
                // TODO: make a noise
                mTransmitter = null;
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
        mNotification = mBuilder.build();
        
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
    }

    @Override
    public void onDestroy() {
        mClient.terminate();
        super.onDestroy();
    }
}
