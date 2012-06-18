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
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.daemon.Preferences;

public class DaemonService extends Service {
	
	private final String TAG = "DaemonService";
	
	private Object _notification_lock = new Object();
	private boolean _notification_dirty = false;
	private Integer _notification_last_size = 0;
	
	private ExecutorService _executor = null;
	
	public static final String ACTION_STARTUP = "de.tubs.ibr.dtn.action.STARTUP";
	public static final String ACTION_SHUTDOWN = "de.tubs.ibr.dtn.action.SHUTDOWN";
	
    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final DTNService.Stub mBinder = new DTNService.Stub() {  	
		@Override
		public DaemonState getState() throws RemoteException {
			return DaemonManager.getInstance().getState();
		}

		@Override
		public boolean isRunning() throws RemoteException {
			return DaemonManager.getInstance().isRunning();
		}

		@Override
		public List<String> getLog() throws RemoteException {
			return DaemonManager.getInstance().getLog();
		}

		@Override
		public List<String> getNeighbors() throws RemoteException {
			return DaemonManager.getInstance().getNeighbors();
		}

		@Override
		public void clearStorage() throws RemoteException {
			DaemonManager.getInstance().clearStorage();
		}

		@Override
		public DTNSession getSession(String packageName) throws RemoteException {
			ClientSession cs = DaemonManager.getInstance().getSession(packageName);
			if (cs == null) return null;
			return cs.getBinder();
		}
    };

	@Override
	public IBinder onBind(Intent intent)
	{
		return mBinder;
	}
	
	private BroadcastReceiver _event_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.NEIGHBOR)) {
				updateNeighborNotification();
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
			if (!DaemonManager.getInstance().getState().equals(DaemonState.ONLINE)) return;
			
			synchronized(_notification_lock) {
				if (!_notification_dirty) return;
				_notification_dirty = false;
			}

			// state is online
			Log.i(TAG, "Query neighbors");
			List<String> neighbors = DaemonManager.getInstance().getNeighbors();
			
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
		Notification ret = new Notification(icon, getResources().getString(R.string.service_name), 0);
		ret.flags = Notification.FLAG_ONGOING_EVENT | Notification.FLAG_NO_CLEAR;		
		
		Intent notifyIntent = new Intent(this, Preferences.class);
		notifyIntent.setAction("android.intent.action.MAIN");
		notifyIntent.addCategory("android.intent.category.LAUNCHER");  
		
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notifyIntent, 0);
		ret.setLatestEventInfo(this, getResources().getString(R.string.service_name), text, contentIntent);
		
		return ret;
	}
	
	@Override
	public void onCreate()
	{
		super.onCreate();
		
		// create a new executor
		_executor = Executors.newSingleThreadExecutor();
		
		// terminate old running instances
		_executor.execute(new Runnable() {
			@Override
			public void run() {
				DaemonManager.getInstance().reset(DaemonService.this);
			}
		});
		
		// listen on daemon event broadcasts
		IntentFilter event_filter = new IntentFilter(de.tubs.ibr.dtn.Intent.NEIGHBOR);
		event_filter.addCategory(Intent.CATEGORY_DEFAULT);
  		registerReceiver(_event_receiver, event_filter );
		
		if (Log.isLoggable("IBR-DTN", Log.DEBUG)) Log.d(TAG, "DaemonService created");
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
        if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Received start id " + startId + ": " + intent);
        
        if (intent == null) return START_NOT_STICKY;
        
        // get the action to do
        String action = intent.getAction();
        
        // if no action is set, just start as not sticky
        if (action == null) return START_NOT_STICKY;
        
        if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Action: " + action);
        
        // copy startId to a final variable
        final int stopId = startId;
        
        if (action.equals(ACTION_STARTUP))
        {
			// create initial notification
			Notification n = buildNotification(R.drawable.ic_notification, getResources().getString(R.string.service_running));
			
    		// turn this to a foreground service (kill-proof)
    		startForeground(1, n);
    		
    		_executor.execute(new Runnable() {
    	        public void run() {
    	    		// start the daemon
    	    		if (DaemonManager.getInstance().start(DaemonService.this) )
    	    		{
        	    		// broadcast state changed intent
        	    		invoke_state_changed_intent();
        	    		
        	    		// update notification icon
        	    		updateNeighborNotification();
    	    		}
    	    		else
    	    		{
        	    		// broadcast state changed intent
        	    		invoke_state_changed_intent();
        	    		
        	    		// update notification icon
        	    		updateNeighborNotification();
        	    		
    	    			// stop the service
        	    		stopSelfResult(stopId);
    	    		}
    	        }
    		});
    		
            return START_STICKY;
        }
        else if (action.equals(ACTION_SHUTDOWN))
        {
        	_executor.execute(new Runnable() {
    	        public void run() {
    	    		// start the daemon
    	    		DaemonManager.getInstance().stop();
    	    		
    	    		// broadcast state changed intent
    	    		invoke_state_changed_intent();
    	    		
    	    		// stop foreground service
    	        	stopForeground(true);
    	    		
    	    		// remove notification
    	    		NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
    	    		nm.cancel(1);
    	    		
    	    		// stop the service
    	    		stopSelfResult(stopId);
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
    	        	DaemonManager.getInstance().getSessionManager().register(DaemonService.this, pi.getTargetPackage(), reg);
    	    		
    	        	if (!DaemonManager.getInstance().isRunning()) {
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
    	        	DaemonManager.getInstance().getSessionManager().unregister(DaemonService.this, pi.getTargetPackage());
    	    		
    	        	if (!DaemonManager.getInstance().isRunning()) {
	    	    		// stop the service
	    	    		stopSelfResult(stopId);
    	        	}
    	        }
    		});
        	
            return START_STICKY;
		}

        // return as not sticky if no one need another behavior
		return START_NOT_STICKY;
	}

	@Override
	public void onDestroy() {
		// unregister intent receiver
		unregisterReceiver(_event_receiver);
		
    	_executor.execute(new Runnable() {
	        public void run() {
	    		// stop daemon if still running
	    		DaemonManager.getInstance().stop();
	        }
		});
		
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
		
		// call super method
		super.onDestroy();
	}
	
	private void invoke_state_changed_intent()
	{
		// broadcast shutdown intent
		Intent broadcastIntent = new Intent();
		broadcastIntent.setAction(de.tubs.ibr.dtn.Intent.STATE);
		broadcastIntent.putExtra("state", DaemonManager.getInstance().getState().name());	
		broadcastIntent.addCategory(Intent.CATEGORY_DEFAULT);
		sendBroadcast(broadcastIntent);
	}
}
