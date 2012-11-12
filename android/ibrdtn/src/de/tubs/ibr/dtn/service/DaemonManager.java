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
import de.tubs.ibr.dtn.service.DaemonProcess.ProcessListener;

public class DaemonManager implements ProcessListener {
	// CloudUplink Parameter
	private static final SingletonEndpoint __CLOUD_EID__ = new SingletonEndpoint("dtn://cloud.dtnbone.dtn");
	private static final String __CLOUD_PROTOCOL__ = "tcp";
	private static final String __CLOUD_ADDRESS__ = "134.169.35.130"; //quorra.ibr.cs.tu-bs.de";
	private static final String __CLOUD_PORT__ = "4559";
	
	// local context for event listener
	private Context _context = null;
	
	// tag for debugging
	private final String TAG = "DaemonManager";
	
	// local object for the daemon process
	private DaemonProcess _process = null;
	
	private DaemonController _controller = null;
	
	// event client for event reception
	private EventClient _event_client = null;
	
	// state of the daemon
	private DaemonState _state = DaemonState.UNKOWN;
	
	// state of the cloud uplink
	private Boolean _cloud_uplink_initiated = false;
	
	// report events to the DaemonService
	private DaemonStateListener _daemon_state_listener = null;

	// interface for the daemon states
	public static interface DaemonStateListener {
		public void onDaemonStateChanged(DaemonState state);
		public void onNeighborhoodChanged();
	}
	
	public DaemonManager(Context context) {
		this._context = context;
	}
	
	// close control connection to the daemon
	public void destroy()
	{
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
	
	// create a new API connection to the daemon
	public APIConnection getAPIConnection() {
		if (_process == null) return null;
		return _process.getAPIConnection();
	}
	
	public synchronized boolean isRunning()
	{
		//if (_process == null) return false;
		//if (_state.equals(DaemonState.UNKOWN)) return _process.isRunning();
		return (_state.equals(DaemonState.ONLINE));
		
		//return _process.isRunning();
	}
	
	public DaemonState getState()
	{
		return _state;
	}
	
	private synchronized void setState(DaemonState state)
	{
		if (_state.equals(state)) return;
		
		Log.i(TAG, "Daemon state changed: " + state.name());
		_state = state;
		
		if (_daemon_state_listener != null) {
			_daemon_state_listener.onDaemonStateChanged(_state);
		}
	}
	
	private EventListener _listener = new EventListener()
	{
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
		
		_process = new DaemonProcess(context, this);
		_process.kill();
	}
	
	public synchronized void start(Context context, DaemonStateListener listener)
	{
        // activate the daemon
		if (isRunning()) return;
		
		_daemon_state_listener = listener;
		
		// save the context
    	_context = context;
		
		if (_process == null)
		{
			// create a new process
	    	_process = new DaemonProcess(context, this);
		}
		
		// startup the daemon
		_process.start(context);
	}

	public synchronized void stop()
	{
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
    	
    	if (_controller != null) {
    		_controller.terminate();
    		_controller = null;
    	}
    	
    	// shutdown the daemon
    	if (_process != null) {
	    	_process.kill();
	    	_process = null;
    	}
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
	
	public synchronized List<String> getLog()
	{
		if (_controller != null)
		{
			return _controller.getLog();
		}
		
		return new LinkedList<String>();
	}
	
	public synchronized List<String> getNeighbors()
	{
		if (_controller != null)
		{
			return _controller.getNeighbors();
		}
		
		return new LinkedList<String>();
	}
	
	public synchronized void enableCloudUplink() {
		if (_controller != null)
		{
			if (!_cloud_uplink_initiated)
			{
				_controller.addConnection(__CLOUD_EID__, __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
				_cloud_uplink_initiated = true;
			}
		}
	}
	
	public synchronized void disableCloudUplink() {
		if (_controller != null)
		{
			if (_cloud_uplink_initiated)
			{
				_controller.removeConnection(__CLOUD_EID__, __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
				_cloud_uplink_initiated = false;
			}
		}
	}
	
	private Runnable _channel_setup_task = new Runnable() {

		public void run() {
			if (_event_client == null)
			{
	            // wait until the daemon is ready
	            try {
	            	EventClient ec = new EventClient(_listener);
	            	
		            while (!ec.isConnected()) {
		            	if (_process == null) {
							onProcessError();
							return;
		            	}
		            	
		    			ec.setConnection( _process.getAPIConnection() );
		    			
		    			try {
		    				ec.open();
		    				break;
		    			} catch (UnknownHostException e) {
							onProcessError();
							return;
		    			} catch (IOException e) {
    						Thread.sleep(1000);
    					}
		            }
		            
		            _event_client = ec;
				} catch (InterruptedException e) {
					onProcessError();
					return;
				}
			}
			
			// create a daemon controller
			_controller = new DaemonController(_process.getAPIConnection());
			
			// enable-cloud uplink if configured
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(DaemonManager.this._context);
			if (prefs.getBoolean("cloud_uplink", false))
			{
				_controller.addConnection(__CLOUD_EID__, __CLOUD_PROTOCOL__, __CLOUD_ADDRESS__, __CLOUD_PORT__);
				_cloud_uplink_initiated = true;
			}
			
			// broadcast startup intent
			setState(DaemonState.ONLINE);
		}
	};

	public void onProcessStart() {
		// daemon is starting...
	}

	public void onProcessStop() {
		// do not overwrite a error state
		if (getState().equals(DaemonState.ERROR)) return;
		
		// broadcast shutdown intent
		setState(DaemonState.OFFLINE);
	}

	public void onProcessError() {
		// stop all connections
		stop();
		
		// broadcast error intent
		setState(DaemonState.ERROR);
	}

	public void onProcessLog(String log) {
		// is daemon ready to serve API connections?
		if (log.contains("INFO: API initialized")) {
			Thread cst = new Thread(_channel_setup_task);
			cst.start();
		}
		// are there changes to the neighborhood
		else if (log.contains("NodeEvent")) {
			if (this._daemon_state_listener != null) {
				this._daemon_state_listener.onNeighborhoodChanged();
			}
		}

//		if (log.contains("Unable to bind to")) {
//		}
	}
}
