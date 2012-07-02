/*
 * DaemonManager.java
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

import ibrdtn.api.APIConnection;
import ibrdtn.api.Event;
import ibrdtn.api.EventClient;
import ibrdtn.api.EventListener;
import ibrdtn.api.object.SingletonEndpoint;

import java.io.File;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.Log;
import de.tubs.ibr.dtn.DaemonState;

public class DaemonManager {
	// CloudUplink Parameter
	private static final SingletonEndpoint __CLOUD_EID__ = new SingletonEndpoint("dtn://cloud.dtnbone.dtn");
	private static final String __CLOUD_PROTOCOL__ = "tcp";
	private static final String __CLOUD_ADDRESS__ = "134.169.35.130"; //quorra.ibr.cs.tu-bs.de";
	private static final String __CLOUD_PORT__ = "4559";
	
	// reference to a singleton instance
	private static final DaemonManager _this = new DaemonManager();
	
	// local context for event listener
	private Context _context = null;
	
	// tag for debugging
	private final String TAG = "DaemonManager";
	
	// local object for the daemon process
	private DaemonProcess _process = null;
	
	// event client for event reception
	private EventClient _event_client = null;
	
	// state of the daemon
	private DaemonState _state = DaemonState.UNKOWN;
	
	// state of the cloud uplink
	private Boolean _cloud_uplink_initiated = false;
	
	// session manager for all active sessions
	private SessionManager _session_manager = new SessionManager();
	
	public DaemonManager()
	{
	}
	
	public void destroy()
	{
		// close all sessions
		_session_manager.destroySessions();
		
		if (_event_client != null)
		{
			try {
				_event_client.close();
			} catch (IOException e) {
			} finally {
				_event_client = null;
			}
		}
	}
	
	public static DaemonManager getInstance()
	{
		return _this;
	}
	
	public SessionManager getSessionManager()
	{
		return _session_manager;
	}
	
	public ClientSession getSession(String packageName)
	{
		return _session_manager.getSession(packageName);
	}
	
	public synchronized boolean isRunning()
	{
		if (_process == null) return false;
		//if (_state.equals(DaemonState.UNKOWN)) return _process.isRunning();
		//return (_state.equals(DaemonState.ONLINE));
		
		return _process.isRunning();
	}
	
	public DaemonState getState()
	{
		return _state;
	}
	
	private synchronized void setState(DaemonState state)
	{
		Log.i(TAG, "Daemon state changed: " + state.name());
		_state = state;
		
		if (_state.equals(DaemonState.OFFLINE))
		{
			_session_manager.terminate();
		}
	}
	
	private EventListener _listener = new EventListener()
	{
		@Override
		public void eventRaised(Event evt)
		{
	        Intent event = new Intent(de.tubs.ibr.dtn.Intent.EVENT);
	        Intent neighborIntent = null;
	        
	        event.addCategory(Intent.CATEGORY_DEFAULT);
	        event.putExtra("name", evt.getName());
	        
	        if (evt.getName().equals("NodeEvent")) {
	        	neighborIntent = new Intent(de.tubs.ibr.dtn.Intent.NEIGHBOR);
	        	neighborIntent.addCategory(Intent.CATEGORY_DEFAULT);
	        }
	        
	        // place the action into the intent
	        if (evt.getAction() != null)
	        {
	        	event.putExtra("action", evt.getAction());
	        	
	        	if (neighborIntent != null) {
		        	if (evt.getAction().equals("available")) {
		        		neighborIntent.putExtra("action", "available");
		        	}
		        	else if (evt.getAction().equals("unavailable")) {
		        		neighborIntent.putExtra("action", "unavailable");
		        	}
		        	else {
		        		neighborIntent = null;
		        	}
	        	}
	        }
	        
	        // put all attributes into the intent
	        for (Map.Entry<String, String> entry : evt.getAttributes().entrySet())
	        {
	        	event.putExtra("attr:" + entry.getKey(), entry.getValue());
	        	if (neighborIntent != null) {
	        		neighborIntent.putExtra("attr:" + entry.getKey(), entry.getValue());
	        	}
	        }
	        
			// send event intent
	        _context.sendBroadcast(event);
	        
	        if (neighborIntent != null) {
	        	_context.sendBroadcast(neighborIntent);
	        }
	        
	        if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "EVENT intent broadcasted: " + evt.getName() + "; Action: " + evt.getAction());
		}
	};
	
	/**
	 * Resets a eventually running daemon without a connection
	 * to any process.
	 */
	public synchronized void reset(Context context)
	{
		// if the process is known as running, just stop the process
		if (isRunning()) stop();
		
		_process = new DaemonProcess(context);
		_process.kill();
	}
	
	public synchronized Boolean start(Context context)
	{
        // activate the daemon
		if (isRunning()) return true;
		
		if (_process == null)
		{
			// create a new process
	    	_process = new DaemonProcess(context);
		}
		
		// restore registrations
		_session_manager.restoreRegistrations(context);
		
		// startup the daemon
		_process.start(context);
        
		if (_event_client == null)
		{
            // wait until the daemon is ready - up to 30 seconds
            try {
	            for (int i = 0; i < 30; i++)
	            {
	            	if (_process.isRunning()) break;
					Thread.sleep(1000);
					
					if (i == 29)
					{
						// broadcast error intent
						setState(DaemonState.ERROR);
						
						Log.e(TAG, "could not start event connection to the daemon due timeout");
						return false;
					}
	            }
			} catch (InterruptedException e) {
				// broadcast error intent
				setState(DaemonState.ERROR);
				return false;
			}
            
            try {
	    		// create a new event client
            	_context = context;
    			_event_client = new EventClient(_listener);
    			_event_client.setConnection( _process.getAPIConnection() );
				_event_client.open();
			} catch (UnknownHostException e) {
				// mark this task as finished
				Log.e(TAG, "could not start event connection to the daemon: " + e.toString());
				
				// broadcast error intent
				setState(DaemonState.ERROR);
				return false;
			} catch (IOException e) {
				// mark this task as finished
				Log.e(TAG, "could not start event connection to the daemon: " + e.toString());
				
				// broadcast error intent
				setState(DaemonState.ERROR);
				return false;
			}
		}
		
		// broadcast startup intent
		setState(DaemonState.ONLINE);
		
		// enable-cloud uplink if configured
    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this._context);
		if (prefs.getBoolean("cloud_uplink", false))
		{
			_process.addConnection(__CLOUD_EID__, __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
			_cloud_uplink_initiated = true;
		}
		
		// fire up the session mananger
		_session_manager.initialize(context);
		
		return true;
	}

	public synchronized void stop()
	{
		// close all sessions
		_session_manager.destroySessions();
		
    	if (_event_client != null)
    	{
    		try {
    			_event_client.close();
			} catch (IOException e) {
				Log.e(TAG, "could not close event connection to the daemon: " + e.toString());
			} finally {
				_event_client = null;
			}
    	}
    	
    	// shutdown the daemon
    	if (_process != null) {
	    	_process.kill();
	    	_process = null;
    	}
    	
		// broadcast shutdown intent
		setState(DaemonState.OFFLINE);
	}
	
	public synchronized void clearStorage()
	{
    	// check if the daemon is running
    	if (isRunning()) return;
        	
		// clear the storage directory
		File bundles = DaemonManager.getStoragePath("bundles");
		if (bundles == null) return;
		
		// flush storage path
		File[] files = bundles.listFiles();
		if (files != null)
		{
			for (File f : files)
			{
				f.delete();
			}
		}
	}
	
	public static File getStoragePath(String subdir)
	{
		if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED))
		{
			File externalStorage = Environment.getExternalStorageDirectory();
			return new File(externalStorage.getPath() + File.separatorChar + "ibrdtn" + File.separatorChar + subdir);
		}
		
		return null;
	}
	
	public APIConnection getDaemonConnection()
	{
		// TODO: throw exception is daemon is not running
		if (_process == null) return null;
		return _process.getAPIConnection();
	}

	public synchronized List<String> getLog()
	{
		if (_process != null)
		{
			if (_process.isRunning())
			{
				return _process.getLog();
			}
		}
		
		return new LinkedList<String>();
	}
	
	public synchronized List<String> getNeighbors()
	{
		if (_process != null)
		{
			if (_process.isRunning())
			{
				return _process.getNeighbors();
			}
		}
		
		return new LinkedList<String>();
	}
	
	public synchronized void enableCloudUplink() {
		if (_process != null)
		{
			if (_process.isRunning() && (!_cloud_uplink_initiated))
			{
				_process.addConnection(__CLOUD_EID__, __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
				_cloud_uplink_initiated = true;
			}
		}
	}
	
	public synchronized void disableCloudUplink() {
		if (_process != null)
		{
			if (_process.isRunning() && _cloud_uplink_initiated)
			{
				_process.removeConnection(__CLOUD_EID__, __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
				_cloud_uplink_initiated = false;
			}
		}
	}
}
