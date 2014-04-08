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

import java.util.Calendar;
import java.util.Date;
import java.util.List;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.support.v4.app.TaskStackBuilder;
import android.util.Log;
import android.widget.Toast;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.Node;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.daemon.NeighborActivity;
import de.tubs.ibr.dtn.daemon.Preferences;
import de.tubs.ibr.dtn.daemon.api.SelectNeighborActivity;
import de.tubs.ibr.dtn.stats.ConvergenceLayerStatsEntry;
import de.tubs.ibr.dtn.stats.StatsDatabase;
import de.tubs.ibr.dtn.stats.StatsEntry;
import de.tubs.ibr.dtn.swig.DaemonRunLevel;
import de.tubs.ibr.dtn.swig.NativeStats;
import de.tubs.ibr.dtn.swig.StringVec;

public class DaemonService extends Service {
	private static final String ACTION_INITIALIZE = "de.tubs.ibr.dtn.action.INITIALIZE";

	public static final String ACTION_STARTUP = "de.tubs.ibr.dtn.action.STARTUP";
	public static final String ACTION_SHUTDOWN = "de.tubs.ibr.dtn.action.SHUTDOWN";
	public static final String ACTION_RESTART = "de.tubs.ibr.dtn.action.RESTART";

	public static final String ACTION_NETWORK_CHANGED = "de.tubs.ibr.dtn.action.NETWORK_CHANGED";

	public static final String ACTION_UPDATE_NOTIFICATION = "de.tubs.ibr.dtn.action.UPDATE_NOTIFICATION";
	public static final String ACTION_INITIATE_CONNECTION = "de.tubs.ibr.dtn.action.INITIATE_CONNECTION";
	public static final String ACTION_CLEAR_STORAGE = "de.tubs.ibr.dtn.action.CLEAR_STORAGE";

	public static final String ACTION_STORE_STATS = "de.tubs.ibr.dtn.action.STORE_STATS";

	public static final String ACTION_START_DISCOVERY = "de.tubs.ibr.dtn.action.START_DISCOVERY";
	public static final String ACTION_STOP_DISCOVERY = "de.tubs.ibr.dtn.action.STOP_DISCOVERY";
	public static final String EXTRA_DISCOVERY_DURATION = "de.tubs.ibr.dtn.intent.DISCOVERY_DURATION";

	public static final String PREFERENCE_NAME = "de.tubs.ibr.dtn.service_prefs";

	private final String TAG = "DaemonService";

	private volatile Looper mServiceLooper;
	private volatile ServiceHandler mServiceHandler;
	
	private int MSG_WHAT_STOP_DISCOVERY = 1;

	// session manager for all active sessions
	private SessionManager mSessionManager = null;

	// the P2P manager used for wifi direct control
	private P2pManager mP2pManager = null;

	// the daemon process
	private DaemonProcess mDaemonProcess = null;

	// indicates if a notification is visible
	private Boolean mShowNotification = false;

	// statistic database
	private StatsDatabase mStatsDatabase = null;
	private Date mStatsLastAction = null;
	
	// global discovery state
	private Boolean mDiscoveryState = false;

	// This is the object that receives interactions from clients. See
	// RemoteService for a more complete example.
	private final DTNService.Stub mBinder = new LocalDTNService();

	public class LocalDTNService extends DTNService.Stub {
		@Override
		public DaemonState getState() throws RemoteException {
			return DaemonService.this.mDaemonProcess.getState();
		}

		@Override
		public List<Node> getNeighbors() throws RemoteException {
			return DaemonService.this.mDaemonProcess.getNeighbors();
		}

		@Override
		public DTNSession getSession(String sessionKey) throws RemoteException {
			int caller = Binder.getCallingUid();
			String[] packageNames = DaemonService.this.getPackageManager()
					.getPackagesForUid(caller);

			ClientSession cs = mSessionManager.getSession(packageNames, sessionKey);
			if (cs == null)
				return null;
			return cs.getBinder();
		}

		@Override
		public String[] getVersion() throws RemoteException {
			return DaemonService.this.mDaemonProcess.getVersion();
		}

		@Override
		public String getEndpoint() throws RemoteException {
			return Preferences.getEndpoint(DaemonService.this);
		}

		public DaemonService getLocal() {
			return DaemonService.this;
		}

		@Override
		public Bundle getSelectNeighborIntent() throws RemoteException {
			Bundle ret = new Bundle();
			Intent intent = new Intent(DaemonService.this, SelectNeighborActivity.class);
			PendingIntent pi = PendingIntent.getActivity(DaemonService.this, 0, intent, PendingIntent.FLAG_ONE_SHOT);
			ret.putParcelable(de.tubs.ibr.dtn.Intent.EXTRA_PENDING_INTENT, pi);
			return ret;
		}
	};

	public boolean isP2pActive() {
		if (mP2pManager == null)
			return false;
		return mP2pManager.isActive();
	}

	public NativeStats getStats() {
		return mDaemonProcess.getStats();
	}

	public StatsDatabase getStatsDatabase() {
		return mStatsDatabase;
	}

	private Runnable mCollectStats = new Runnable() {
		@Override
		public void run() {
			final Intent storeStatsIntent = new Intent(DaemonService.this, DaemonService.class);
			storeStatsIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STORE_STATS);
			startService(storeStatsIntent);
		}
	};

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
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
		}
	}

	/**
	 * Incoming Intents are handled here
	 * 
	 * @param intent
	 */
	@SuppressWarnings("deprecation")
	public void onHandleIntent(Intent intent, int startId) {
		String action = intent.getAction();

		if (ACTION_STARTUP.equals(action)) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

			// only start if the daemon is offline or in error state
			// and if the daemon switch is on
			if ((mDaemonProcess.getState().equals(DaemonState.OFFLINE) || mDaemonProcess.getState()
					.equals(DaemonState.ERROR))
					&& prefs.getBoolean(Preferences.KEY_ENABLED, true)) {
				// start-up the daemon
				mDaemonProcess.start();

				final Intent storeStatsIntent = new Intent(this, DaemonService.class);
				storeStatsIntent
						.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STORE_STATS);
				startService(storeStatsIntent);
			}
		} else if (ACTION_SHUTDOWN.equals(action)) {
			// stop main loop
			mDaemonProcess.stop();
		} else if (ACTION_RESTART.equals(action)) {
			Integer level = intent.getIntExtra("runlevel", 0);

			// restart the daemon into the given runlevel
			mDaemonProcess.restart(level, new DaemonProcess.OnRestartListener() {
				@Override
				public void OnStop(DaemonRunLevel previous, DaemonRunLevel next) {
					if (next.swigValue() < DaemonRunLevel.RUNLEVEL_API.swigValue() && previous.swigValue() >= DaemonRunLevel.RUNLEVEL_API.swigValue()) {
						// shutdown the session manager
						mSessionManager.destroy();
					}
				}

				@Override
				public void OnStart(DaemonRunLevel previous, DaemonRunLevel next) {
					if (previous.swigValue() < DaemonRunLevel.RUNLEVEL_API.swigValue() && next.swigValue() >= DaemonRunLevel.RUNLEVEL_API.swigValue()) {
						// re-initialize the session manager
						mSessionManager.initialize();
					}
				}

				@Override
				public void OnReloadConfiguration() {
				}
			});

			final Intent storeStatsIntent = new Intent(this, DaemonService.class);
			storeStatsIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STORE_STATS);
			startService(storeStatsIntent);
		} else if (ACTION_UPDATE_NOTIFICATION.equals(action)) {
			if (intent.hasExtra("text")) {
				// update state text in the notification
				updateNotification(intent.getStringExtra("text"));
			} else {
				// update state text in the notification
				updateNotification(null);
			}
		} else if (de.tubs.ibr.dtn.Intent.REGISTER.equals(action)) {
			final Registration reg = (Registration) intent.getParcelableExtra("registration");
			final PendingIntent pi = (PendingIntent) intent.getParcelableExtra("app");

			mSessionManager.register(pi.getTargetPackage(), reg);

		} else if (de.tubs.ibr.dtn.Intent.UNREGISTER.equals(action)) {
			final PendingIntent pi = (PendingIntent) intent.getParcelableExtra("app");

			mSessionManager.unregister(pi.getTargetPackage());
		} else if (ACTION_INITIATE_CONNECTION.equals(action)) {
			if (intent.hasExtra("endpoint")) {
				mDaemonProcess.initiateConnection(intent.getStringExtra("endpoint"));
			}
		} else if (ACTION_CLEAR_STORAGE.equals(action)) {
			mDaemonProcess.clearStorage();
			Toast.makeText(this, R.string.toast_storage_cleared, Toast.LENGTH_SHORT).show();
		} else if (ACTION_NETWORK_CHANGED.equals(action)) {
			// This intent tickle the service if something has changed in the
			// network configuration. If the service was enabled but terminated
			// before,
			// it will be started now.

			final Intent storeStatsIntent = new Intent(this, DaemonService.class);
			storeStatsIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STORE_STATS);
			startService(storeStatsIntent);

			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

			// if discovery is configured as "smart"
			if ("smart".equals(prefs.getString(Preferences.KEY_DISCOVERY_MODE, "smart"))) {
				// enable discovery for 2 minutes
				final Intent discoIntent = new Intent(DaemonService.this, DaemonService.class);
				discoIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_DISCOVERY);
				discoIntent.putExtra(EXTRA_DISCOVERY_DURATION, 120L);
				startService(discoIntent);
			}
		} else if (ACTION_STORE_STATS.equals(action)) {
			// cancel the next scheduled collection
			mServiceHandler.removeCallbacks(mCollectStats);

			if (mStatsLastAction != null) {
				Calendar now = Calendar.getInstance();
				now.roll(Calendar.MINUTE, -1);
				if (mStatsLastAction.before(now.getTime())) {
					refreshStats();
				}
			} else {
				refreshStats();
			}

			// schedule next collection in 15 minutes
			mServiceHandler.postDelayed(mCollectStats, 900000);
		} else if (ACTION_INITIALIZE.equals(action)) {
			// initialize the daemon service
			initialize();
		} else if (ACTION_START_DISCOVERY.equals(action)) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			String discoMode = prefs.getString(Preferences.KEY_DISCOVERY_MODE, "smart");
			boolean stayOn = "on".equals(discoMode) && mDiscoveryState;
			
			// create stop discovery intent
			Intent stopIntent = new Intent(this, DaemonService.class);
			stopIntent.setAction(DaemonService.ACTION_STOP_DISCOVERY);
			
			if (intent.hasExtra(EXTRA_DISCOVERY_DURATION) && !stayOn) {
				Long duration = intent.getLongExtra(EXTRA_DISCOVERY_DURATION, 120);
				
				Message msg = mServiceHandler.obtainMessage();
				msg.what = MSG_WHAT_STOP_DISCOVERY;
				msg.arg1 = startId;
				msg.obj = stopIntent;
				mServiceHandler.sendMessageDelayed(msg, duration * 1000);

				Log.i(TAG, "Discovery stop scheduled in " + duration + " seconds.");
			}
			else {
				mServiceHandler.removeMessages(MSG_WHAT_STOP_DISCOVERY);
				Log.i(TAG, "Scheduled discovery stop removed.");
			}

			// only start discovery if not active
			if (!mDiscoveryState) {
				// start P2P discovery and enable IPND
				mDaemonProcess.startDiscovery();
	
				// start Wi-Fi P2P discovery
				if (mP2pManager != null)
					mP2pManager.startDiscovery();
				
				// set global discovery state
				mDiscoveryState = true;
				
				// request notification update
				requestNotificationUpdate(getResources().getString(R.string.ticker_discovery_started));
			}
		} else if (ACTION_STOP_DISCOVERY.equals(action)) {
			// only stop discovery if active
			if (mDiscoveryState) {
				// stop P2P discovery and disable IPND
				mDaemonProcess.stopDiscovery();
	
				// stop Wi-Fi P2P discovery
				if (mP2pManager != null)
					mP2pManager.stopDiscovery();
				
				// set global discovery state
				mDiscoveryState = false;
				
				// request notification update
				requestNotificationUpdate(getResources().getString(R.string.ticker_discovery_stopped));
			}
			
			// remove all stop discovery messages
			mServiceHandler.removeMessages(MSG_WHAT_STOP_DISCOVERY);
			Log.i(TAG, "Scheduled discovery stop removed.");
		}

		// stop the daemon if it should be offline
		if (mDaemonProcess.getState().equals(DaemonState.OFFLINE) && (startId != -1))
			stopSelf(startId);
	}

	private void refreshStats() {
		NativeStats nstats = mDaemonProcess.getStats();

		// query the daemon stats and store them in the database
		mStatsDatabase.put(new StatsEntry(nstats));

		StringVec tags = nstats.getTags();
		for (int i = 0; i < tags.size(); i++) {
			ConvergenceLayerStatsEntry e = new ConvergenceLayerStatsEntry(nstats, tags.get(i), i);
			mStatsDatabase.put(e);
		}

		mStatsLastAction = new Date();
	}

	public DaemonService() {
		super();
	}

	@Override
	public void onCreate() {
		super.onCreate();

		// open statistic database
		mStatsDatabase = new StatsDatabase(this);
		mStatsLastAction = null;

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
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
			mP2pManager = new P2pManager(this);
			mP2pManager.create();
		}

		// start initialization of the daemon process
		final Intent intent = new Intent(this, DaemonService.class);
		intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_INITIALIZE);

		// queue the initialization job as the first job of the handler
		Message msg = mServiceHandler.obtainMessage();
		msg.arg1 = -1; // invalid startId (this never leads to a stop of the
						// service)
		msg.obj = intent;
		mServiceHandler.sendMessage(msg);
	}

	/**
	 * Initialize the daemon service This should be the first job of the service
	 * after creation
	 */
	private void initialize() {
		// initialize the basic daemon
		mDaemonProcess.initialize();

		// restore registrations
		mSessionManager.initialize();

		if (Log.isLoggable(TAG, Log.DEBUG))
			Log.d(TAG, "DaemonService created");

		// start daemon if enabled
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

		// listen to preference changes
		prefs.registerOnSharedPreferenceChangeListener(_pref_listener);

		if (prefs.getBoolean(Preferences.KEY_ENABLED, true)) {
			// startup the daemon process
			final Intent intent = new Intent(this, DaemonService.class);
			intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STARTUP);
			startService(intent);
		}
	}

	/**
	 * Called on stopSelf() or stopService()
	 */
	@Override
	public void onDestroy() {
		// unlisten to preference changes
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
		prefs.unregisterOnSharedPreferenceChangeListener(_pref_listener);

		// disable P2P manager
		if (mP2pManager != null)
			mP2pManager.destroy();

		// stop looper that handles incoming intents
		mServiceLooper.quit();

		// close all sessions
		mSessionManager.destroy();

		// shutdown daemon completely
		mDaemonProcess.destroy();
		mDaemonProcess = null;

		// dereference P2P Manager
		mP2pManager = null;

		// remove notification
		NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
		nm.cancel(1);

		// close statistic database
		mStatsDatabase.close();

		// call super method
		super.onDestroy();
	}

	@Override
	public void onStart(Intent intent, int startId) {
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

	private DaemonProcessHandler mProcessHandler = new DaemonProcessHandler() {

		@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
		@Override
		public void onStateChanged(DaemonState state) {
			Log.d(TAG, "DaemonState: " + state);

			// prepare broadcast intent
			Intent broadcastIntent = new Intent();
			broadcastIntent.setAction(de.tubs.ibr.dtn.Intent.STATE);
			broadcastIntent.putExtra("state", state.name());
			broadcastIntent.addCategory(Intent.CATEGORY_DEFAULT);

			// request notification update
			requestNotificationUpdate(null);

			SharedPreferences prefs = PreferenceManager
					.getDefaultSharedPreferences(DaemonService.this);

			switch (state) {
				case ERROR:
					break;

				case OFFLINE:
					if (prefs.getBoolean(Preferences.KEY_P2P_ENABLED, true)) {
						if (mP2pManager != null)
							mP2pManager.setEnabled(false);
					}

					// disable foreground service only if the daemon has been
					// switched off
					if (!prefs.getBoolean(Preferences.KEY_ENABLED, true)) {
						// mark the notification as invisible
						mShowNotification = false;

						// stop foreground service
						stopForeground(true);

						// stop service
						stopSelf();
					}

					// if discovery is configured as some kind of active
					if (!"off".equals(prefs.getString(Preferences.KEY_DISCOVERY_MODE, "smart"))) {
						// disable discovery
						final Intent discoIntent = new Intent(DaemonService.this,
								DaemonService.class);
						discoIntent
								.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STOP_DISCOVERY);
						startService(discoIntent);
					}

					break;

				case ONLINE:
					// mark the notification as visible
					mShowNotification = true;

					// create initial notification
					Notification n = buildNotification(null);

					// turn this to a foreground service (kill-proof)
					startForeground(1, n);

					if (prefs.getBoolean(Preferences.KEY_P2P_ENABLED, false)) {
						if (mP2pManager != null)
							mP2pManager.setEnabled(true);
					}

					if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR1) {
						// wake-up all apps in stopped-state when going online
						broadcastIntent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
					}

					// if discovery is configured as "smart"
					if ("smart".equals(prefs.getString(Preferences.KEY_DISCOVERY_MODE, "smart"))) {
						// enable discovery for 2 minutes
						final Intent discoIntent = new Intent(DaemonService.this,
								DaemonService.class);
						discoIntent
								.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_DISCOVERY);
						discoIntent.putExtra(EXTRA_DISCOVERY_DURATION, 120L);
						startService(discoIntent);
					}
					// if discovery is configured as "on"
					else if ("on".equals(prefs.getString(Preferences.KEY_DISCOVERY_MODE, "smart"))) {
						// enable discovery
						final Intent discoIntent = new Intent(DaemonService.this,
								DaemonService.class);
						discoIntent
								.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_DISCOVERY);
						startService(discoIntent);
					}
					break;

				case SUSPENDED:
					break;
				case UNKOWN:
					break;
				default:
					break;

			}

			// broadcast state change
			sendBroadcast(broadcastIntent);
		}

		@Override
		public void onNeighborhoodChanged() {
			requestNotificationUpdate(null);
		}

		@Override
		public void onEvent(Intent intent) {
			sendBroadcast(intent);
		}

	};

	private void requestNotificationUpdate(String text) {
		// request notification update
		final Intent i = new Intent(DaemonService.this, DaemonService.class);
		i.setAction(ACTION_UPDATE_NOTIFICATION);
		
		if (text != null) {
			i.putExtra("text", text);
		}
		
		startService(i);
	}

	private void updateNotification(String stateText) {
		NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		// update the notification only if it is visible
		if (mShowNotification) {
			nm.notify(1, buildNotification(stateText));
		}
	}

	@SuppressWarnings("deprecation")
	private Notification buildNotification(String stateText) {
		String content = Preferences.getEndpoint(DaemonService.this);

		NotificationCompat.Builder builder = new NotificationCompat.Builder(this);

		builder.setContentTitle(getResources().getString(R.string.service_name));
		builder.setContentText(content);

		builder.setSmallIcon(R.drawable.ic_notification);
		builder.setOngoing(true);
		builder.setOnlyAlertOnce(true);
		builder.setWhen(0);
		if (stateText != null) builder.setTicker(stateText);

		List<Node> neighbors = mDaemonProcess.getNeighbors();
		builder.setNumber(neighbors.size());

		// create intent for the neighbor list
		Intent showNeighborsIntent = new Intent(this, NeighborActivity.class);
		showNeighborsIntent.setAction("android.intent.action.MAIN");
		showNeighborsIntent.addCategory("android.intent.category.LAUNCHER");
		
		if (mDiscoveryState) {
			// create intent to stop discovery
			Intent stopDiscoveryIntent = new Intent(this, DaemonService.class);
			stopDiscoveryIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STOP_DISCOVERY);
			PendingIntent piDisco = PendingIntent.getService(this, 0, stopDiscoveryIntent, 0);
			builder.addAction(R.drawable.ic_action_discovery_stop, getString(R.string.stop_discovery), piDisco);
		} else {
			// create intent to start discovery
			Intent startDiscoveryIntent = new Intent(this, DaemonService.class);
			startDiscoveryIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_DISCOVERY);
			
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			if ("smart".equals(prefs.getString(Preferences.KEY_DISCOVERY_MODE, "smart"))) {
				startDiscoveryIntent.putExtra(DaemonService.EXTRA_DISCOVERY_DURATION, 120L);
			}
			
			PendingIntent piDisco = PendingIntent.getService(this, 0, startDiscoveryIntent, 0);
			builder.addAction(R.drawable.ic_action_discovery, getString(R.string.start_discovery), piDisco);
		}
		
		TaskStackBuilder stackBuilder = TaskStackBuilder.create(this);
		stackBuilder.addParentStack(NeighborActivity.class);
		stackBuilder.addNextIntent(showNeighborsIntent);
		PendingIntent contentIntent = stackBuilder.getPendingIntent(0, PendingIntent.FLAG_UPDATE_CURRENT); // PendingIntent.getActivity(this, 0, showNeighborsIntent, 0);
		builder.setContentIntent(contentIntent);
		

		return builder.getNotification();
	}

	private SharedPreferences.OnSharedPreferenceChangeListener _pref_listener = new SharedPreferences.OnSharedPreferenceChangeListener() {

		@Override
		public void onSharedPreferenceChanged(
				SharedPreferences sharedPreferences, String key) {

			if (Preferences.KEY_P2P_ENABLED.equals(key)) {
				if (sharedPreferences.getBoolean(key, false)
						&& mDaemonProcess.getState().equals(DaemonState.ONLINE)) {
					if (mP2pManager != null)
						mP2pManager.setEnabled(true);
				} else {
					if (mP2pManager != null)
						mP2pManager.setEnabled(false);
				}
			} else if (Preferences.KEY_DISCOVERY_MODE.equals(key)) {
				final String disco_mode = sharedPreferences.getString(key, "smart");

				// if discovery is configured as "on"
				if ("on".equals(disco_mode)) {
					// enable discovery
					final Intent discoIntent = new Intent(DaemonService.this, DaemonService.class);
					discoIntent
							.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_DISCOVERY);
					startService(discoIntent);
				}
				// if discovery is configured as "off"
				else if ("off".equals(disco_mode)) {
					// disable discovery
					final Intent discoIntent = new Intent(DaemonService.this, DaemonService.class);
					discoIntent
							.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STOP_DISCOVERY);
					startService(discoIntent);
				}
				// if discovery is configured as "smart"
				else if ("smart".equals(disco_mode)) {
					// enable discovery for 2 minutes
					final Intent discoIntent = new Intent(DaemonService.this, DaemonService.class);
					discoIntent
							.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_START_DISCOVERY);
					discoIntent.putExtra(EXTRA_DISCOVERY_DURATION, 120L);
					startService(discoIntent);
				}
			}
		}

	};
}
