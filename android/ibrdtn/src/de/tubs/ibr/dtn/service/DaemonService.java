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

import java.util.Arrays;
import java.util.List;

import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import de.tubs.ibr.dtn.daemon.Preferences;
import de.tubs.ibr.dtn.p2p.P2PManager;
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

public class DaemonService extends Service {
	public static final String ACTION_STARTUP = "de.tubs.ibr.dtn.action.STARTUP";
	public static final String ACTION_SHUTDOWN = "de.tubs.ibr.dtn.action.SHUTDOWN";
	public static final String ACTION_CLOUD_UPLINK = "de.tubs.ibr.dtn.action.CLOUD_UPLINK";
	public static final String QUERY_NEIGHBORS = "de.tubs.ibr.dtn.action.QUERY_NEIGHBORS";

	// CloudUplink Parameter
	private static final SingletonEndpoint __CLOUD_EID__ = new SingletonEndpoint("dtn://cloud.dtnbone.dtn");
	private static final String __CLOUD_PROTOCOL__ = "tcp";
	private static final String __CLOUD_ADDRESS__ = "134.169.35.130"; // quorra.ibr.cs.tu-bs.de";
	private static final String __CLOUD_PORT__ = "4559";

	private final String TAG = "DaemonService";

	private volatile Looper mServiceLooper;
	private volatile ServiceHandler mServiceHandler;

	private Object _notification_lock = new Object();
	private boolean _notification_dirty = false;
	private Integer _notification_last_size = 0;

	// session manager for all active sessions
	private SessionManager _session_manager = null;

	// the P2P manager used for wifi direct control
	private P2PManager _p2p_manager = null;

	private DaemonMainThread mDaemonMainThread;

	public DaemonState getState()
	{
		if (mDaemonMainThread != null && mDaemonMainThread.isAlive())
		{
			return DaemonState.ONLINE;
		} else
		{
			return DaemonState.OFFLINE;
		}
	}

	// This is the object that receives interactions from clients. See
	// RemoteService for a more complete example.
	private final DTNService.Stub mBinder = new DTNService.Stub() {
		public DaemonState getState() throws RemoteException
		{
			return DaemonService.this.getState();
		}

		public boolean isRunning() throws RemoteException
		{
			return DaemonService.this.getState().equals(DaemonState.ONLINE);
		}

		public List<String> getNeighbors() throws RemoteException
		{
			String[] neighbors = NativeDaemonWrapper.getNeighbors();

			return Arrays.asList(neighbors);
		}

		public void clearStorage() throws RemoteException
		{
			DaemonStorageUtils.clearStorage();
		}

		public DTNSession getSession(String packageName) throws RemoteException
		{
			ClientSession cs = _session_manager.getSession(packageName);
			if (cs == null) return null;
			return cs.getBinder();
		}

		@Override
		public List<String> getLog() throws RemoteException
		{
			// TODO: recompile ibr-dtn-api.jar. This method is removed in
			// current
			// DTNService aidl
			return null;
		}
	};

	@Override
	public IBinder onBind(Intent intent)
	{
		//TODO: Do we really need to start the service when binding to it? This seems redundant.
		// start the service if enabled and not running
		// if (!getState().equals(DaemonState.ONLINE))
		// {
		// SharedPreferences prefs =
		// PreferenceManager.getDefaultSharedPreferences(this);
		// if (prefs.getBoolean("enabledSwitch", false))
		// {
		// Log.d(TAG, "Startup daemon on bind");
		// // startup the daemon process
		// final Intent startUpIntent = new Intent(this, DaemonService.class);
		// startUpIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STARTUP);
		// startService(startUpIntent);
		// }
		// }

		return mBinder;
	}

	private void updateNeighborNotification()
	{
		synchronized (_notification_lock)
		{
			if (_notification_dirty) return;
			_notification_dirty = true;

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
		public void onReceive(Context context, Intent intent)
		{
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.STATE))
			{
				String state = intent.getStringExtra("state");
				DaemonState ds = DaemonState.valueOf(state);
				switch (ds)
				{
				case ONLINE:
					Log.d(TAG, "mDaemonStateReceiver: DaemonState: ONLINE");

					NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
					nm.notify(1, buildNotification(R.drawable.ic_notification, getResources().getString(R.string.notify_no_neighbors)));

					// restore registrations
					_session_manager.initialize();

					// update notification icon
					updateNeighborNotification();

					// enable P2P manager
//					_p2p_manager.initialize();
					break;

				case OFFLINE:
					Log.d(TAG, "mDaemonStateReceiver: DaemonState: OFFLINE");

					// stop service
					stopSelf();
					break;

				case ERROR:

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
		public void handleMessage(Message msg)
		{
			onHandleIntent((Intent) msg.obj);
		}
	}

	/**
	 * Incoming Intents are handled here
	 * 
	 * @param intent
	 */
	public void onHandleIntent(Intent intent)
	{
		String action = intent.getAction();

		if (ACTION_STARTUP.equals(action))
		{
			// create initial notification
			Notification n = buildNotification(R.drawable.ic_notification, getResources().getString(R.string.dialog_wait_starting));

			// turn this to a foreground service (kill-proof)
			startForeground(1, n);

			mDaemonMainThread = new DaemonMainThread(this);
			mDaemonMainThread.start();
			Log.d(TAG, "Started mDaemonMainThread");

		} else if (ACTION_SHUTDOWN.equals(action))
		{
			NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
			nm.notify(1, buildNotification(R.drawable.ic_notification, getResources().getString(R.string.dialog_wait_stopping)));

			// stop main loop
			NativeDaemonWrapper.daemonShutdown();

			// disable P2P manager
//			_p2p_manager.destroy();

			// close all sessions
			_session_manager.terminate();

			// stop foreground service
			stopForeground(true);

			// remove notification
			nm.cancel(1);

		} else if (ACTION_CLOUD_UPLINK.equals(action))
		{
			if (intent.hasExtra("enabled"))
			{
				if (intent.getBooleanExtra("enabled", false))
				{
					NativeDaemonWrapper.addConnection(__CLOUD_EID__.toString(), __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
				} else
				{
					NativeDaemonWrapper.removeConnection(__CLOUD_EID__.toString(), __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
				}
			}
		} else if (QUERY_NEIGHBORS.equals(action))
		{
			if (!getState().equals(DaemonState.ONLINE)) return;

			synchronized (_notification_lock)
			{
				if (!_notification_dirty) return;
				_notification_dirty = false;
			}

			// state is online
			Log.i(TAG, "Query neighbors");
			String[] neighborsArray = NativeDaemonWrapper.getNeighbors();
			List<String> neighbors = Arrays.asList(neighborsArray);

			synchronized (_notification_lock)
			{
				if (_notification_last_size.equals(neighbors.size())) return;
				_notification_last_size = neighbors.size();
			}

			NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
			Notification n = null;

			if (neighbors.size() > 0)
			{
				// _notification =
				// buildNotification(R.drawable.ic_notification_active,
				// getResources().getString(R.string.notify_neighbors) + ": " +
				// count);
				n = buildNotification(R.drawable.ic_notification, getResources().getString(R.string.notify_neighbors) + ": " + neighbors.size());
			} else
			{
				n = buildNotification(R.drawable.ic_notification, getResources().getString(R.string.notify_no_neighbors));
			}

			nm.notify(1, n);
		} else if (de.tubs.ibr.dtn.Intent.REGISTER.equals(action))
		{
			final Registration reg = (Registration) intent.getParcelableExtra("registration");
			final PendingIntent pi = (PendingIntent) intent.getParcelableExtra("app");

			_session_manager.register(pi.getTargetPackage(), reg);

		} else if (de.tubs.ibr.dtn.Intent.UNREGISTER.equals(action))
		{
			final PendingIntent pi = (PendingIntent) intent.getParcelableExtra("app");

			_session_manager.unregister(pi.getTargetPackage());
		}

	}

	public DaemonService() {
		super();
	}

	@Override
	public void onCreate()
	{
		super.onCreate();

		IntentFilter ifilter = new IntentFilter(de.tubs.ibr.dtn.Intent.STATE);
		ifilter.addCategory(Intent.CATEGORY_DEFAULT);
		registerReceiver(mDaemonStateReceiver, ifilter);

		/*
		 * incoming Intents will be processed by ServiceHandler and queued in
		 * HandlerThread
		 */
		HandlerThread thread = new HandlerThread("DaemonService_IntentThread");
		thread.start();
		mServiceLooper = thread.getLooper();
		mServiceHandler = new ServiceHandler(mServiceLooper);

		// create a session manager
		_session_manager = new SessionManager(this);

		// create P2P Manager
//		_p2p_manager = new P2PManager(this, _p2p_listener, "my address");

		if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "DaemonService created");

		// restore sessions
		_session_manager.restoreRegistrations();
	}

	/**
	 * Called on stopSelf() or stopService()
	 */
	@Override
	public void onDestroy()
	{

		unregisterReceiver(mDaemonStateReceiver);

		// stop looper that handles incoming intents
		mServiceLooper.quit();

		// close all sessions
		_session_manager.saveRegistrations();

		// remove notification
		NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
		nm.cancel(1);

		// dereference P2P Manager
		_p2p_manager = null;

		// call super method
		super.onDestroy();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId)
	{
		/*
		 * If no explicit intent is given start as ACTION_STARTUP.
		 * 
		 * When this service crashes, Android restarts it without an Intent.
		 * Thus ACTION_STARTUP is executed!
		 */
		if (intent == null || intent.getAction() == null)
		{
			Log.d(TAG, "intent == null or intent.getAction() == null -> default to ACTION_STARTUP");

			intent = new Intent(ACTION_STARTUP);
		}

		String action = intent.getAction();

		if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Received start id " + startId + ": " + intent);
		if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Intent Action: " + action);

		if (ACTION_STARTUP.equals(action))
		{
			// handle startup intent directly without queuing
			if (!getState().equals(DaemonState.ONLINE))
			{
				onHandleIntent(intent);
			}
		} else
		{
			// otherwise, queue intents to work them off ordered in own
			// threads
			if (getState().equals(DaemonState.ONLINE))
			{
				Message msg = mServiceHandler.obtainMessage();
				msg.arg1 = startId;
				msg.obj = intent;
				mServiceHandler.sendMessage(msg);
			} else
			{
				Log.e(TAG, "Main loop of daemon is not running!");
				stopSelf();
			}
		}

		return START_STICKY;
	}

	// TODO: Not used currently!
	public void onNeighborhoodChanged()
	{
		updateNeighborNotification();
	}

//	private P2PManager.P2PNeighborListener _p2p_listener = new P2PManager.P2PNeighborListener() {
//
//		public void onNeighborDisconnected(String name, String iface)
//		{
//			Log.d(TAG, "P2P neighbor has been disconnected");
//			// TODO: put here the right code to control the dtnd
//		}
//
//		public void onNeighborDisappear(String name)
//		{
//			Log.d(TAG, "P2P neighbor has been disappeared");
//			// TODO: put here the right code to control the dtnd
//		}
//
//		public void onNeighborDetected(String name)
//		{
//			Log.d(TAG, "P2P neighbor has been detected");
//			// TODO: put here the right code to control the dtnd
//		}
//
//		public void onNeighborConnected(String name, String iface)
//		{
//			Log.d(TAG, "P2P neighbor has been connected");
//			// TODO: put here the right code to control the dtnd
//		}
//	};

	private Notification buildNotification(int icon, String text)
	{
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
