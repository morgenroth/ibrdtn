/*
 * DaemonService.java
 * 
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: Dominik Schürmann <dominik@dominikschuermann.de>
 * 	           Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package de.tubs.ibr.dtn.service;

import java.util.LinkedList;
import java.util.List;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.daemon.Preferences;
import de.tubs.ibr.dtn.p2p.P2PManager;
import de.tubs.ibr.dtn.swig.StringVec;

public class DaemonService extends Service {
    public static final String ACTION_STARTUP = "de.tubs.ibr.dtn.action.STARTUP";
    public static final String ACTION_SHUTDOWN = "de.tubs.ibr.dtn.action.SHUTDOWN";
    public static final String ACTION_CLOUD_UPLINK = "de.tubs.ibr.dtn.action.CLOUD_UPLINK";
    public static final String UPDATE_NOTIFICATION = "de.tubs.ibr.dtn.action.UPDATE_NOTIFICATION";

    // CloudUplink Parameter
    private static final SingletonEndpoint __CLOUD_EID__ = new SingletonEndpoint(
            "dtn://cloud.dtnbone.dtn");
    private static final String __CLOUD_PROTOCOL__ = "tcp";
    private static final String __CLOUD_ADDRESS__ = "134.169.35.130"; // quorra.ibr.cs.tu-bs.de";
    private static final String __CLOUD_PORT__ = "4559";

    private final String TAG = "DaemonService";

    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;

    // session manager for all active sessions
    private SessionManager mSessionManager = null;

    // the P2P manager used for wifi direct control
    private P2PManager _p2p_manager = null;

    private DaemonProcess mDaemonProcess = null;

    // This is the object that receives interactions from clients. See
    // RemoteService for a more complete example.
    private final DTNService.Stub mBinder = new DTNService.Stub() {
        public DaemonState getState() throws RemoteException {
            return DaemonService.this.mDaemonProcess.getState();
        }

        public boolean isRunning() throws RemoteException {
            return DaemonService.this.mDaemonProcess.getState().equals(DaemonState.ONLINE);
        }

        public List<String> getNeighbors() throws RemoteException {
            if (mDaemonProcess == null)
                return new LinkedList<String>();

            List<String> ret = new LinkedList<String>();
            StringVec neighbors = mDaemonProcess.getNative().getNeighbors();
            for (int i = 0; i < neighbors.size(); i++) {
                ret.add(neighbors.get(i));
            }

            return ret;
        }

        public void clearStorage() throws RemoteException {
            DaemonStorageUtils.clearStorage();
        }

        public DTNSession getSession(String packageName) throws RemoteException {
            ClientSession cs = mSessionManager.getSession(packageName);
            if (cs == null)
                return null;
            return cs.getBinder();
        }

        @Override
        public String[] getVersion() throws RemoteException {
            StringVec version = mDaemonProcess.getNative().getVersion();
            return new String[] { version.get(0), version.get(1) };
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Intent intent = (Intent) msg.obj;
            onHandleIntent(intent);
        }
    }

    /**
     * Incoming Intents are handled here
     * 
     * @param intent
     */
    public void onHandleIntent(Intent intent) {
        String action = intent.getAction();

        if (ACTION_STARTUP.equals(action)) {
            // create initial notification
            Notification n = buildNotification(R.drawable.ic_notification, getResources()
                    .getString(R.string.notify_pending));

            // turn this to a foreground service (kill-proof)
            startForeground(1, n);

            mDaemonProcess.start();
        } else if (ACTION_SHUTDOWN.equals(action)) {
            // stop main loop
            mDaemonProcess.stop();

            // stop foreground service
            stopForeground(true);
        } else if (ACTION_CLOUD_UPLINK.equals(action)) {
            if (DaemonState.ONLINE.equals(mDaemonProcess.getState())) {
                setCloudUplink(intent.getBooleanExtra("enabled", false));
            }
        } else if (UPDATE_NOTIFICATION.equals(action)) {
            // update state text in the notification 
            updateNotification();
        } else if (de.tubs.ibr.dtn.Intent.REGISTER.equals(action)) {
            final Registration reg = (Registration) intent.getParcelableExtra("registration");
            final PendingIntent pi = (PendingIntent) intent.getParcelableExtra("app");

            mSessionManager.register(pi.getTargetPackage(), reg);

        } else if (de.tubs.ibr.dtn.Intent.UNREGISTER.equals(action)) {
            final PendingIntent pi = (PendingIntent) intent.getParcelableExtra("app");

            mSessionManager.unregister(pi.getTargetPackage());
        }

    }

    public DaemonService() {
        super();
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // create daemon main thread
        mDaemonProcess = new DaemonProcess(this, mProcessHandler);

        /*
         * incoming Intents will be processed by ServiceHandler and queued in
         * HandlerThread
         */
        HandlerThread thread = new HandlerThread("DaemonService_IntentThread");
        thread.start();
        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);

        // create a session manager
        mSessionManager = new SessionManager(this);

        // create P2P Manager
        // _p2p_manager = new P2PManager(this, _p2p_listener, "my address");

        if (Log.isLoggable(TAG, Log.DEBUG))
            Log.d(TAG, "DaemonService created");

        // restore sessions
        mSessionManager.restoreRegistrations();
    }

    /**
     * Called on stopSelf() or stopService()
     */
    @Override
    public void onDestroy() {
        // stop looper that handles incoming intents
        mServiceLooper.quit();

        // close all sessions
        mSessionManager.saveRegistrations();

        // remove notification
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        nm.cancel(1);

        // dereference P2P Manager
        _p2p_manager = null;

        // call super method
        super.onDestroy();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        /*
         * If no explicit intent is given start as ACTION_STARTUP. When this
         * service crashes, Android restarts it without an Intent. Thus
         * ACTION_STARTUP is executed!
         */
        if (intent == null || intent.getAction() == null) {
            Log.d(TAG, "intent == null or intent.getAction() == null -> default to ACTION_STARTUP");

            intent = new Intent(ACTION_STARTUP);
        }

        String action = intent.getAction();

        if (Log.isLoggable(TAG, Log.DEBUG))
            Log.d(TAG, "Received start id " + startId + ": " + intent);
        if (Log.isLoggable(TAG, Log.DEBUG))
            Log.d(TAG, "Intent Action: " + action);

        if (ACTION_STARTUP.equals(action)) {
            // handle startup intent directly without queuing
            if (mDaemonProcess.getState().equals(DaemonState.OFFLINE))
            {
                onHandleIntent(intent);
            }
        } else {
            Message msg = mServiceHandler.obtainMessage();
            msg.arg1 = startId;
            msg.obj = intent;
            mServiceHandler.sendMessage(msg);
        }

        return START_STICKY;
    }

    private DaemonProcessHandler mProcessHandler = new DaemonProcessHandler() {

        @Override
        public void onStateChanged(DaemonState state) {
            Log.d(TAG, "mDaemonStateReceiver: DaemonState: " + state);

            switch (state) {
                case ERROR:
                    break;
                    
                case OFFLINE:
                    // close all sessions
                    mSessionManager.terminate();
                    
                    // TODO: disable P2P manager
                    // _p2p_manager.destroy();
                    
                    // stop service
                    stopSelf();
                    break;
                    
                case ONLINE:
                    // update notification text
                    updateNotification();

                    // restore registrations
                    mSessionManager.initialize();

                    // request notification update
                    requestNotificationUpdate();

                    // TODO: enable P2P manager
                    // _p2p_manager.initialize();
                   
                    // enable cloud uplink if enabled
                    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DaemonService.this);
                    setCloudUplink(prefs.getBoolean("enabledSwitch", false));
                    break;
                    
                case SUSPENDED:
                    break;
                case UNKOWN:
                    break;
                default:
                    break;
                
            }
            
            // broadcast state change
            Intent broadcastIntent = new Intent();
            broadcastIntent.setAction(de.tubs.ibr.dtn.Intent.STATE);
            broadcastIntent.putExtra("state", state.name());
            broadcastIntent.addCategory(Intent.CATEGORY_DEFAULT);
            sendBroadcast(broadcastIntent);
        }

        @Override
        public void onNeighborhoodChanged() {
            requestNotificationUpdate();
        }

        @Override
        public void onEvent(Intent intent) {
            sendBroadcast(intent);
        }
        
    };
    
    private void setCloudUplink(Boolean mode) {
        if (mode) {
            mDaemonProcess.getNative().addConnection(__CLOUD_EID__.toString(),
                    __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
        } else {
            mDaemonProcess.getNative().removeConnection(__CLOUD_EID__.toString(),
                    __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
        }
    }
    
    private void requestNotificationUpdate() {
        // request notification update
        final Intent neighborIntent = new Intent(DaemonService.this, DaemonService.class);
        neighborIntent.setAction(UPDATE_NOTIFICATION);
        startService(neighborIntent);
    }
    
    private void updateNotification() {
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        String stateText = "";

        // check state and display daemon state instead of neighbors
        switch (this.mDaemonProcess.getState()) {
            case PENDING:
                stateText = getResources().getString(R.string.notify_pending) + " ...";
                break;
            case ERROR:
                stateText = getResources().getString(R.string.notify_error);
                break;
            case OFFLINE:
                stateText = getResources().getString(R.string.notify_offline);
                break;
            case ONLINE:
                // if the daemon is online, query for the number of neighbors and display it
                StringVec neighbors = mDaemonProcess.getNative().getNeighbors();
        
                if (neighbors.size() > 0) {
                    stateText = getResources().getString(R.string.notify_neighbors) + ": " + neighbors.size();
                } else {
                    stateText = getResources().getString(R.string.notify_no_neighbors);
                }
                break;
            case SUSPENDED:
                stateText = getResources().getString(R.string.notify_suspended);
                break;
            case UNKOWN:
                break;
            default:
                break;
        }
        
        nm.notify(1, buildNotification(R.drawable.ic_notification, stateText));
    }

    // private P2PManager.P2PNeighborListener _p2p_listener = new
    // P2PManager.P2PNeighborListener() {
    //
    // public void onNeighborDisconnected(String name, String iface)
    // {
    // Log.d(TAG, "P2P neighbor has been disconnected");
    // // TODO: put here the right code to control the dtnd
    // }
    //
    // public void onNeighborDisappear(String name)
    // {
    // Log.d(TAG, "P2P neighbor has been disappeared");
    // // TODO: put here the right code to control the dtnd
    // }
    //
    // public void onNeighborDetected(String name)
    // {
    // Log.d(TAG, "P2P neighbor has been detected");
    // // TODO: put here the right code to control the dtnd
    // }
    //
    // public void onNeighborConnected(String name, String iface)
    // {
    // Log.d(TAG, "P2P neighbor has been connected");
    // // TODO: put here the right code to control the dtnd
    // }
    // };

    private Notification buildNotification(int icon, String text) {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this);

        builder.setContentTitle(getResources().getString(R.string.service_name));
        builder.setContentText(text);
        builder.setSmallIcon(icon);
        builder.setOngoing(true);
        builder.setOnlyAlertOnce(true);
        builder.setWhen(0);

        Intent notifyIntent = new Intent(this, Preferences.class);
        notifyIntent.setAction("android.intent.action.MAIN");
        notifyIntent.addCategory("android.intent.category.LAUNCHER");

        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notifyIntent, 0);
        builder.setContentIntent(contentIntent);

        return builder.getNotification();
    }

}
