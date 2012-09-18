/*
 * DaemonService.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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

import ibrdtn.api.ManageClient;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.daemon.Preferences;
import de.tubs.ibr.dtn.p2p.P2PManager;
import de.tubs.ibr.dtn.service.DaemonManager.DaemonStateListener;

public class DaemonService extends Service implements DaemonStateListener {
	
	private final String TAG = "DaemonService";
	
	private Object _notification_lock = new Object();
	private boolean _notification_dirty = false;
	private Integer _notification_last_size = 0;
	
	private ExecutorService _executor = null;
	
	// control object for the launched daemon
	private DaemonManager _daemon = null;
	
	// session manager for all active sessions
	private SessionManager _session_manager = null;
	
	// the P2P manager used for wifi direct control
	private P2PManager _p2p_manager = null;
	
	public static final String ACTION_STARTUP = "de.tubs.ibr.dtn.action.STARTUP";
	public static final String ACTION_SHUTDOWN = "de.tubs.ibr.dtn.action.SHUTDOWN";
	public static final String ACTION_CLOUD_UPLINK = "de.tubs.ibr.dtn.action.CLOUD_UPLINK";
	
    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final DTNService.Stub mBinder = new DTNService.Stub() {  	
		@Override
		public DaemonState getState() throws RemoteException {
			return _daemon.getState();
		}

		@Override
		public boolean isRunning() throws RemoteException {
			return _daemon.getState().equals(DaemonState.ONLINE);
		}

		@Override
		public List<String> getLog() throws RemoteException {
			return _daemon.getLog();
		}

		@Override
		public List<String> getNeighbors() throws RemoteException {
			return _daemon.getNeighbors();
		}

		@Override
		public void clearStorage() throws RemoteException {
			_daemon.clearStorage();
		}

		@Override
		public DTNSession getSession(String packageName) throws RemoteException {
			ClientSession cs = _session_manager.getSession(packageName);
			if (cs == null) return null;
			return cs.getBinder();
		}
    };

	@Override
	public IBinder onBind(Intent intent)
	{
		// start the service if enabled and not running
		if (!_daemon.getState().equals(DaemonState.ONLINE)) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			if (prefs.getBoolean("enabledSwitch", false)) {
				// startup the daemon process
				final Intent startUpIntent = new Intent(this, DaemonService.class);
				startUpIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STARTUP);
				startService(startUpIntent);
			}
		}
		
		return mBinder;
	}
	
	private BroadcastReceiver _intent_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (DaemonService.ACTION_CLOUD_UPLINK.equals(intent.getAction())) {
				if (intent.hasExtra("enabled")) {
					if (intent.getBooleanExtra("enabled", false)) {
						_executor.execute(new Runnable() {
							@Override
							public void run() {
								_daemon.enableCloudUplink();
							}
						});
					} else {
						_executor.execute(new Runnable() {
							@Override
							public void run() {
								_daemon.disableCloudUplink();
							}
						});
					}
				}
			}
		}
	};
	
	private void updateNeighborNotification() {
		synchronized(_notification_lock) {
			if (_notification_dirty) return;
			_notification_dirty = true;
			_executor.execute(_unn_task);
		}
	}
	
	private Runnable _unn_task = new Runnable() {
		@Override
		public void run() {
			if (!_daemon.getState().equals(DaemonState.ONLINE)) return;
			
			synchronized(_notification_lock) {
				if (!_notification_dirty) return;
				_notification_dirty = false;
			}

			// state is online
			Log.i(TAG, "Query neighbors");
			List<String> neighbors = _daemon.getNeighbors();
			
			synchronized(_notification_lock) {
				if (_notification_last_size.equals(neighbors.size())) return;
				_notification_last_size = neighbors.size();
			}
	
			NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
			Notification n = null;
			
			if (neighbors.size() > 0) {
				//_notification = buildNotification(R.drawable.ic_notification_active, getResources().getString(R.string.notify_neighbors) + ": " + count);
				n = buildNotification(R.drawable.ic_notification, getResources().getString(R.string.notify_neighbors) + ": " + neighbors.size());
			} else {
				n = buildNotification(R.drawable.ic_notification, getResources().getString(R.string.notify_no_neighbors));
			}
			
			nm.notify(1, n);
		}
	};
	
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
	
	@Override
	public void onCreate()
	{
		super.onCreate();
		
		// create a controllable daemon object
		_daemon = new DaemonManager(this);
		
		// create a session manager
		_session_manager = new SessionManager(this, _daemon);
		
		// create P2P Manager
		_p2p_manager = new P2PManager(this, _p2p_listener, "my address");
		
		// create a new executor
		_executor = Executors.newSingleThreadExecutor();
		
		IntentFilter cloud_filter = new IntentFilter(DaemonService.ACTION_CLOUD_UPLINK);
		cloud_filter.addCategory(Intent.CATEGORY_DEFAULT);
  		registerReceiver(_intent_receiver, cloud_filter );
		
		if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "DaemonService created");
		
		_executor.execute(new Runnable() {
			@Override
			public void run() {
				// terminate a running daemon
				terminateRunningDaemon();
			}
		});
		
		// restore sessions
		_session_manager.restoreRegistrations();
	}
	
	@Override
	public void onDestroy() {
		// unregister intent receiver
		unregisterReceiver(_intent_receiver);
		
		// close all sessions
		_session_manager.saveRegistrations();
		
		// stop the daemon
    	_daemon.stop();
		
		try {
			// stop executor
			_executor.shutdown();
			
			// ... and wait until all jobs are done
			if (!_executor.awaitTermination(30, TimeUnit.SECONDS)) {
				_executor.shutdownNow();
			}
		} catch (InterruptedException e) {
			Log.e(TAG, "Interrupted on service destruction.", e);
		}
		
		// remove notification
		NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
		nm.cancel(1);
		
		// dereference the daemon object
		_daemon = null;
		
		// dereference P2P Manager
		_p2p_manager = null;
		
		// call super method
		super.onDestroy();
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
        if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Received start id " + startId + ": " + intent);
        
        if (intent == null) return super.onStartCommand(intent, flags, startId);
        
        // get the action to do
        String action = intent.getAction();
        
        // if no action is set, just start as not sticky
        if (action == null) return super.onStartCommand(intent, flags, startId);
        
        if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Action: " + action);
        
        // copy startId to a final variable
        final int stopId = startId;
        
        if (action.equals(ACTION_STARTUP))
        {
			// create initial notification
			Notification n = buildNotification(R.drawable.ic_notification, getResources().getString(R.string.dialog_wait_starting));
			
    		// turn this to a foreground service (kill-proof)
    		startForeground(1, n);
    		
    		_executor.execute(new Runnable() {
				@Override
				public void run() {
					_daemon.start(DaemonService.this, DaemonService.this);
					
					// wait until the daemon is online or in error state
					while ( ! (_daemon.getState().equals(DaemonState.ONLINE) || _daemon.getState().equals(DaemonState.ERROR)) ) {
						try {
							Thread.sleep(1000);
						} catch (InterruptedException e) { }
					}
				}
    		});
    		
            return START_STICKY;
        }
        else if (action.equals(ACTION_SHUTDOWN))
        {
    		_executor.execute(new Runnable() {
				@Override
				public void run() {
					NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
					nm.notify(1, buildNotification(R.drawable.ic_notification, getResources().getString(R.string.dialog_wait_stopping)));
					_daemon.stop();
				}
    		});
        	
            return START_STICKY;
        }
		else if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.REGISTER))
		{
			final Registration reg = (Registration)intent.getParcelableExtra("registration");
			final PendingIntent pi = (PendingIntent)intent.getParcelableExtra("app");
			
        	_executor.execute(new Runnable() {
    	        public void run() {
    	        	_session_manager.register(pi.getTargetPackage(), reg);
    	    		
    	        	if (!_daemon.isRunning()) {
	    	    		// stop the service
	    	    		stopSelfResult(stopId);
    	        	}
    	        }
    		});
        	
            return START_STICKY;
		}
		else if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.UNREGISTER))
		{
			final PendingIntent pi = (PendingIntent)intent.getParcelableExtra("app");
			
        	_executor.execute(new Runnable() {
    	        public void run() {
    	        	_session_manager.unregister(pi.getTargetPackage());
    	    		
    	        	if (!_daemon.isRunning()) {
	    	    		// stop the service
	    	    		stopSelfResult(stopId);
    	        	}
    	        }
    		});
        	
            return START_STICKY;
		}

        // return as not sticky if no one need another behavior
		return super.onStartCommand(intent, flags, startId);
	}
	
	private void invoke_state_changed_intent()
	{
		// broadcast shutdown intent
		Intent broadcastIntent = new Intent();
		broadcastIntent.setAction(de.tubs.ibr.dtn.Intent.STATE);
		if (_daemon == null) {
			broadcastIntent.putExtra("state", DaemonState.OFFLINE.name());
		} else {
			broadcastIntent.putExtra("state", _daemon.getState().name());
		}
		broadcastIntent.addCategory(Intent.CATEGORY_DEFAULT);
		sendBroadcast(broadcastIntent);
	}

	@Override
	public void onDaemonStateChanged(DaemonState state) {
		NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
		
		switch (state) {
		case ERROR:
    		// disable P2P manager
    		_p2p_manager.destroy();
    		
			nm.notify(1, buildNotification(R.drawable.ic_notification, getResources().getString(R.string.notify_error)));
			break;
		case OFFLINE:
    		// disable P2P manager
    		_p2p_manager.destroy();
    		
			// close all sessions
			_session_manager.terminate();
			
    		// broadcast state changed intent
    		invoke_state_changed_intent();
    		
    		// stop foreground service
        	stopForeground(true);
    		
    		// remove notification
    		nm.cancel(1);
			break;
		case ONLINE:
			nm.notify(1, buildNotification(R.drawable.ic_notification, getResources().getString(R.string.notify_no_neighbors)));
			
			// restore registrations
			_session_manager.initialize();
			
			// update notification icon
			updateNeighborNotification();
			
			// enable P2P manager
			_p2p_manager.initialize();
			
			break;
		case SUSPENDED:
			break;
		case UNKOWN:
			break;
		default:
			break;
		
		}
		
		// broadcast state changed intent
		invoke_state_changed_intent();
	}
	
	private void terminateRunningDaemon() {
		ManageClient mc = new ManageClient();
		mc.setConnection(_daemon.getAPIConnection());
		try {
			mc.open();
			mc.shutdown();
			mc.close();
			Log.d(TAG, "shutdown sent to running daemon");
		} catch (UnknownHostException e) {
		} catch (IOException e) {
			Log.d(TAG, "no running daemon found");
		}
	}

	@Override
	public void onNeighborhoodChanged() {
		updateNeighborNotification();
	}
	
	private P2PManager.P2PNeighborListener _p2p_listener = new P2PManager.P2PNeighborListener() {
		
		@Override
		public void onNeighborDisconnected(String name, String iface) {
			Log.d(TAG, "P2P neighbor has been disconnected");
			// TODO: put here the right code to control the dtnd
		}
		
		@Override
		public void onNeighborDisappear(String name) {
			Log.d(TAG, "P2P neighbor has been disappeared");
			// TODO: put here the right code to control the dtnd
		}
		
		@Override
		public void onNeighborDetected(String name) {
			Log.d(TAG, "P2P neighbor has been detected");
			// TODO: put here the right code to control the dtnd
		}
		
		@Override
		public void onNeighborConnected(String name, String iface) {
			Log.d(TAG, "P2P neighbor has been connected");
			// TODO: put here the right code to control the dtnd
		}
	};
}
