/*
 * ClientSession.java
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
import ibrdtn.api.ExtendedClient.APIException;

import java.io.IOException;

import android.content.Context;
import android.content.Intent;
import de.tubs.ibr.dtn.api.BundleID;
import de.tubs.ibr.dtn.api.CallbackMode;
import de.tubs.ibr.dtn.api.DTNSession;
import de.tubs.ibr.dtn.api.DTNSessionCallback;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.api.SessionDestroyedException;
import de.tubs.ibr.dtn.api.SingletonEndpoint;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;

public class ClientSession {
	
	private final static String TAG = "ClientSession";
	
	private String _package_name = null;

	private Context context = null;
    private APISession _session = null;
    private Registration _registration = null;
    
    private Boolean _daemon_online = false;
    
	public ClientSession(Context context, Registration reg, String packageName)
	{
		// create a unique session key
		this.context = context;
		_package_name = packageName;
		_registration = reg;
	}
	
	public synchronized void initialize()
	{
		_daemon_online = true;
		_initialize_process.start();
	}
	
	private Thread _initialize_process = new Thread() {
		@Override
		public void run() {
			try {
				while (!isInterrupted())
				{
					try {
						getSession();
						break;
					} catch (IOException e) {
						wait(5000);
					}
				}
			} catch (InterruptedException e1) {	}
		}
	};
	
	public synchronized void terminate()
	{
		_daemon_online = false;
		_initialize_process.interrupt();
		
		if (_session != null)
		{
			_session.disconnect();
			_session = null;
		}
	}
	
	private synchronized APISession getSession() throws IOException
	{
		if (!_daemon_online) throw new IOException("daemon is offline");
		
		try {
			if (_session == null)
			{
				Log.d(TAG, "try to create an API session with the daemon");
				_session = new APISession(this);
				
				APIConnection socket = DaemonManager.getInstance().getDaemonConnection();
				if (socket == null) throw new IOException("daemon not running");
				
				_session.connect(socket);
				_session.register(_registration);
				
				invoke_registration_intent();
			}
			
			if (_session.isConnected())
			{
				return _session;
			}
			else
			{
				_session = null;
				throw new IOException("not connected");
			}
		} catch (APIException e) {
			_session = null;
			throw new IOException("api error");
		} catch (SessionDestroyedException e) {
			_session = null;
			throw new IOException("not connected");
		}
	}
    
    private final DTNSession.Stub mBinder = new DTNSession.Stub()
    {
		@Override
		public boolean query(DTNSessionCallback cb, CallbackMode mode, BundleID id) throws RemoteException {
			try {
				APISession session = getSession();
				session.query(cb, mode, id);
				return true;
			} catch (Exception e) {
				Log.e(TAG, "query failed", e);
				return false;
			}
		}

		@Override
		public boolean delivered(BundleID id) throws RemoteException {
			try {
				APISession session = getSession();
				session.setDelivered(id);
				return true;
			} catch (Exception e) {
				Log.e(TAG, "delivered failed", e);
				return false;
			}
		}

		@Override
		public boolean queryNext(DTNSessionCallback cb, CallbackMode mode) throws RemoteException {
			try {
				APISession session = getSession();
				return session.query(cb, mode);
			} catch (Exception e) {
				Log.e(TAG, "queryNext failed", e);
				return false;
			}
		}

		@Override
		public boolean send(SingletonEndpoint destination,
				int lifetime, String data) throws RemoteException {
			try {
				APISession session = getSession();
				return session.send(destination, lifetime, data);
			} catch (Exception e) {
				Log.e(TAG, "send failed", e);
				return false;
			}
		}

		@Override
		public boolean sendGroup(GroupEndpoint destination,
				int lifetime, String data) throws RemoteException {
			try {
				APISession session = getSession();
				return session.send(destination, lifetime, data);
			} catch (Exception e) {
				Log.e(TAG, "sendGroup failed", e);
				return false;
			}
		}

		@Override
		public boolean sendFileDescriptor(SingletonEndpoint destination, int lifetime,
				ParcelFileDescriptor fd, long length) throws RemoteException {
			try {
				APISession session = getSession();
				return session.send(destination, lifetime, fd, length);
			} catch (Exception e) {
				Log.e(TAG, "sendFileDescriptor failed", e);
				return false;
			}
		}

		@Override
		public boolean sendGroupFileDescriptor(GroupEndpoint destination, int lifetime,
				ParcelFileDescriptor fd, long length) throws RemoteException {
			try {
				APISession session = getSession();
				return session.send(destination, lifetime, fd, length);
			} catch (Exception e) {
				Log.e(TAG, "sendGroupFileDescriptor failed", e);
				return false;
			}
		}
    };
    
    public DTNSession getBinder()
    {
    	return mBinder;
    }
    
	public String getPackageName()
	{
		return _package_name;
	}
	
	public synchronized void invoke_reconnect()
	{
		_session = null;
	}
	
	public void invoke_receive_intent(BundleID id)
	{
		// forward the notification as intent
		// create a new intent
        Intent notify = new Intent(de.tubs.ibr.dtn.Intent.RECEIVE);
        notify.addCategory(_package_name);
        notify.putExtra("type", "bundle");
        notify.putExtra("data", id);

        // send notification intent
        context.sendBroadcast(notify);
        
        Log.d(TAG, "RECEIVE intent sent to " + _package_name);
	}
	
    private void invoke_registration_intent()
    {
		// send out registration intent to the application
		Intent broadcastIntent = new Intent(de.tubs.ibr.dtn.Intent.REGISTRATION);
		broadcastIntent.addCategory(_package_name);
		broadcastIntent.putExtra("key", _package_name);
		
		// send notification intent
		context.sendBroadcast(broadcastIntent);
		
		Log.d(TAG, "REGISTRATION intent sent to " + _package_name);
    }
}
