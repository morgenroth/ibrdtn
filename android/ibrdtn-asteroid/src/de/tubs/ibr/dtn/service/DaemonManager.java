package de.tubs.ibr.dtn.service;

import ibrdtn.api.APIConnection;
import ibrdtn.api.Event;
import ibrdtn.api.EventClient;
import ibrdtn.api.EventListener;

import java.io.File;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import de.tubs.ibr.dtn.DaemonState;
import android.os.Environment;
import android.util.Log;

public class DaemonManager {
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
	        event.addCategory(Intent.CATEGORY_DEFAULT);
	        event.putExtra("name", evt.getName());
	        
	        // place the action into the intent
	        if (evt.getAction() != null)
	        {
	        	event.putExtra("action", evt.getAction());
	        }
	        
	        // put all attributes into the intent
	        for (Map.Entry<String, String> entry : evt.getAttributes().entrySet())
	        {
	        	event.putExtra("attr:" + entry.getKey(), entry.getValue());
	        }
	        
			// send event intent
	        _context.sendBroadcast(event);
	        
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
}
