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
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
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
    public static final String QUERY_NEIGHBORS = "de.tubs.ibr.dtn.action.QUERY_NEIGHBORS";

    // CloudUplink Parameter
    private static final SingletonEndpoint __CLOUD_EID__ = new SingletonEndpoint(
            "dtn://cloud.dtnbone.dtn");
    private static final String __CLOUD_PROTOCOL__ = "tcp";
    private static final String __CLOUD_ADDRESS__ = "134.169.35.130"; // quorra.ibr.cs.tu-bs.de";
    private static final String __CLOUD_PORT__ = "4559";

    private final String TAG = "DaemonService";

    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;

    private Object mNotificationLock = new Object();
    private boolean mNotificationDirty = false;
    private Long mNotificationLastSize = 0L;

    // session manager for all active sessions
    private SessionManager mSessionManager = null;

    // the P2P manager used for wifi direct control
    private P2PManager _p2p_manager = null;

    private DaemonMainThread mDaemonMainThread = null;

    private final Object mStartingLock = new Object();
    private boolean mStarting = false;

    // This is the object that receives interactions from clients. See
    // RemoteService for a more complete example.
    private final DTNService.Stub mBinder = new DTNService.Stub() {
        public DaemonState getState() throws RemoteException {
            return DaemonService.this.mDaemonMainThread.getState();
        }

        public boolean isRunning() throws RemoteException {
            return DaemonService.this.mDaemonMainThread.getState().equals(DaemonState.ONLINE);
        }

        public List<String> getNeighbors() throws RemoteException {
            if (mDaemonMainThread == null)
                return new LinkedList<String>();

            List<String> ret = new LinkedList<String>();
            StringVec neighbors = mDaemonMainThread.getNative().getNeighbors();
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
            StringVec version = mDaemonMainThread.getNative().getVersion();
            return new String[] { version.get(0), version.get(1) };
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    private void updateNeighborNotification() {
        synchronized (mNotificationLock) {
            if (mNotificationDirty)
                return;
            mNotificationDirty = true;

            // queue query neighbors task
            final Intent neighborIntent = new Intent(this, DaemonService.class);
            neighborIntent.setAction(QUERY_NEIGHBORS);
            startService(neighborIntent);
        }
    }

    /**
     * Gets broadcasted intent when main thread gets online or offline
     */
    private BroadcastReceiver mDaemonStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.STATE)) {
                String state = intent.getStringExtra("state");
                DaemonState ds = DaemonState.valueOf(state);

                Log.d(TAG, "mDaemonStateReceiver: DaemonState: " + state);

                switch (ds) {
                    case ONLINE:
                        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
                        nm.notify(
                                1,
                                buildNotification(R.drawable.ic_notification, getResources()
                                        .getString(R.string.notify_no_neighbors)));

                        // restore registrations
                        mSessionManager.initialize();

                        // update notification icon
                        updateNeighborNotification();

                        // enable P2P manager
                        // _p2p_manager.initialize();

                        synchronized (mStartingLock) {
                            mStarting = false;
                            mStartingLock.notify();
                        }

                        break;

                    case OFFLINE:
                        // stop service
                        stopSelf();
                        break;

                    default:
                        break;
                }
            }
        }
    };

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Intent intent = (Intent) msg.obj;

            // if main daemon thread is currently starting but not finished,
            // wait for it...
            synchronized (mStartingLock) {
                if (mStarting) {
                    Log.d(TAG,
                            "Intent ("
                                    + intent.getAction()
                                    + ") not queued! Daemon is currently starting! Wait for ONLINE state...");
                    try {
                        mStartingLock.wait();
                    } catch (InterruptedException e) {
                        Log.e(TAG, "InterruptedException", e);
                    }
                }
            }

            Log.d(TAG, "ServiceHandler: Now handling Intent " + intent.getAction());

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
            synchronized (mStartingLock) {
                mStarting = true;
            }

            // create initial notification
            Notification n = buildNotification(R.drawable.ic_notification, getResources()
                    .getString(R.string.dialog_wait_starting));

            // turn this to a foreground service (kill-proof)
            startForeground(1, n);

            mDaemonMainThread.start();
        } else if (ACTION_SHUTDOWN.equals(action)) {
            NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
            nm.notify(
                    1,
                    buildNotification(R.drawable.ic_notification,
                            getResources().getString(R.string.dialog_wait_stopping)));

            // close all sessions
            mSessionManager.terminate();
            
            // disable P2P manager
            // _p2p_manager.destroy();
            
            // stop main loop
            mDaemonMainThread.stop();

            // stop foreground service
            stopForeground(true);

            // remove notification
            nm.cancel(1);

        } else if (ACTION_CLOUD_UPLINK.equals(action)) {
            if (intent.hasExtra("enabled")) {
                if (intent.getBooleanExtra("enabled", false)) {
                    mDaemonMainThread.getNative().addConnection(__CLOUD_EID__.toString(),
                            __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
                } else {
                    mDaemonMainThread.getNative().removeConnection(__CLOUD_EID__.toString(),
                            __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
                }
            }
        } else if (QUERY_NEIGHBORS.equals(action)) {
            synchronized (mNotificationLock) {
                if (!mNotificationDirty)
                    return;
                mNotificationDirty = false;
            }

            // state is online
            Log.i(TAG, "Query neighbors");
            StringVec neighbors = mDaemonMainThread.getNative().getNeighbors();

            synchronized (mNotificationLock) {
                if (mNotificationLastSize.equals(neighbors.size()))
                    return;
                mNotificationLastSize = neighbors.size();
            }

            NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
            Notification n = null;

            if (neighbors.size() > 0) {
                // _notification =
                // buildNotification(R.drawable.ic_notification_active,
                // getResources().getString(R.string.notify_neighbors) + ": " +
                // count);
                n = buildNotification(
                        R.drawable.ic_notification,
                        getResources().getString(R.string.notify_neighbors) + ": "
                                + neighbors.size());
            } else {
                n = buildNotification(R.drawable.ic_notification,
                        getResources().getString(R.string.notify_no_neighbors));
            }

            nm.notify(1, n);
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

        IntentFilter ifilter = new IntentFilter(de.tubs.ibr.dtn.Intent.STATE);
        ifilter.addCategory(Intent.CATEGORY_DEFAULT);
        registerReceiver(mDaemonStateReceiver, ifilter);

        // create daemon main thread
        mDaemonMainThread = new DaemonMainThread(this);

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
        unregisterReceiver(mDaemonStateReceiver);

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
            if (mDaemonMainThread.getState().equals(DaemonState.OFFLINE))
            {
                onHandleIntent(intent);
            }
        } else {
            // otherwise, queue intents to work them off ordered in own
            // threads
            synchronized (mStartingLock) {
                if (mDaemonMainThread.getState().equals(DaemonState.ONLINE) || mStarting) {
                    Message msg = mServiceHandler.obtainMessage();
                    msg.arg1 = startId;
                    msg.obj = intent;
                    mServiceHandler.sendMessage(msg);
                } else {
                    Log.e(TAG,
                            "Intent (" + intent.getAction()
                                    + ") discarded! Main thread of daemon is not running!");
                    stopSelf();
                }
            }
        }

        return START_STICKY;
    }

    /**
     * This method is called by the daemon main thread on every change in the
     * node neighborhood.
     */
    public void onNeighborhoodChanged() {
        updateNeighborNotification();
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
