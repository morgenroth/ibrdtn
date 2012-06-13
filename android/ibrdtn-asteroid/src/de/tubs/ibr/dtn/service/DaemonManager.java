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
import ibrdtn.api.ManageClient;
import ibrdtn.api.SocketAPIConnection;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;
import de.tubs.ibr.dtn.DaemonState;

public class DaemonManager {
	// reference to a singleton instance
	private static final DaemonManager _this = new DaemonManager();
	
	// local context for event listener
	private Context _context = null;
	
	// tag for debugging
	private final String TAG = "DaemonManager";
	
	private ManageClient _connection = null;
	
	// event client for event reception
	private EventClient _event_client = null;
	
	// state of the daemon
	private DaemonState _state = DaemonState.UNKOWN;
	
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
		return (_state.equals(DaemonState.ONLINE));
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
	
	public synchronized Boolean start(Context context)
	{
		// restore registrations
		_session_manager.restoreRegistrations(context);
        
		if (_event_client == null)
		{
            try {
	    		// create a new event client
            	_context = context;
    			_event_client = new EventClient(_listener);
    			_event_client.setConnection( getAPIConnection(context) );
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
    	
		try {
			ManageClient c = getConnection();
			Log.i("DaemonProcess", "Process closed.");
			c.close();
		} catch (UnknownHostException e) {
		} catch (IOException e) {
		} finally {
			_connection = null;
		}
    	
		// broadcast shutdown intent
		setState(DaemonState.OFFLINE);
	}
	
	public APIConnection getAPIConnection(Context context) throws IOException
	{
		if (context == null) throw new IOException("no context available");
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		String host = prefs.getString("host_address", "127.0.0.1");
		Integer port = Integer.valueOf( prefs.getString("host_port", "4550") );
		return new SocketAPIConnection(host, port);
	}
	
	private synchronized ManageClient getConnection() throws UnknownHostException, IOException
	{
		// check existing connection first
		if (_connection != null)
		{
			if (_connection.isClosed())
			{
				_connection.close();
				_connection = null;
				Log.i("DaemonProcess", "connection to daemon closed");
			}
		}
		
		if (_connection == null)
		{
			try {
				_connection = new ManageClient();
				_connection.setConnection( getAPIConnection(_context) );
				Log.i("DaemonProcess", "try to connect to daemon");
				_connection.open();
			} catch (UnknownHostException e) {
				_connection = null;
				throw e;
			} catch (IOException e) {
				_connection = null;
				throw e;
			}
		}
		
		return _connection;
	}
	
	public void setSuspend()
	{
		try {
			ManageClient c = getConnection();
			c.suspend();
		} catch (UnknownHostException e) {
		} catch (IOException e) {
		}
	}
	
	public void setResume()
	{
		try {
			ManageClient c = getConnection();
			c.resume();
		} catch (UnknownHostException e) {
		} catch (IOException e) {
		}
	}

	public synchronized List<String> getLog()
	{
		try {
			 ManageClient c = getConnection();
			 return c.getLog();
		} catch (UnknownHostException e1) {
		} catch (IOException e1) {
		}

		return null;
	}
	
	public synchronized List<String> getNeighbors()
	{
		try {
			 ManageClient c = getConnection();
			 return c.getNeighbors();
		} catch (UnknownHostException e1) {
		} catch (IOException e1) {
		}

		return null;
	}
}
