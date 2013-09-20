package de.tubs.ibr.dtn.dtalkie.service;

import android.annotation.SuppressLint;
import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.NotificationCompat;
import de.tubs.ibr.dtn.dtalkie.R;
import de.tubs.ibr.dtn.dtalkie.TalkieActivity;

public class HeadsetService extends Service {
    
    public static final String ENTER_HEADSET_MODE = "de.tubs.ibr.dtn.dtalkie.ENTER_HEADSET_MODE";
    public static final String LEAVE_HEADSET_MODE = "de.tubs.ibr.dtn.dtalkie.LEAVE_HEADSET_MODE";
    
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    
    private Boolean mPersistent = false;

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
            
            // set service mode to persistent
            mPersistent = true;
        }
        else if (LEAVE_HEADSET_MODE.equals(action)) {
            // turn this to a foreground service (kill-proof)
            stopForeground(true);
            
            // set service mode to persistent
            mPersistent = false;
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
}
