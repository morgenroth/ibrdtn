/*
 * DTNClient.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package de.tubs.ibr.dtn.api;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.Parcelable;
import android.os.RemoteException;
import android.util.Log;
import de.tubs.ibr.dtn.DTNService;

public final class DTNClient {
	
	private static final String TAG = "DTNClient";

    // DTN service provided by IBR-DTN
    private DTNService service = null;
    
    // session object
	private Session session = null;
	
	private DataHandler _handler = null;
	
	private Context context = null;
	
	private Registration _registration = null;
	
    private Boolean blocking = true;
    
    private ExecutorService executor = null;
    
    private String _packageName = null;
    
    private SessionConnection _session_handler = null;
    
    public DTNClient(SessionConnection handler) {
    	this._session_handler = handler;
    }
    
    public DTNClient() {
    	// add dummy handler
    	this._session_handler = new SessionConnection() {
			@Override
			public void onSessionConnected(Session session) { }

			@Override
			public void onSessionDisconnected() { }
    	};
    }
	
	/**
	 * If blocking is enabled, the getSession() method will block until a valid session
	 * exists. If set to false, the method would return null if there is no active session.
	 * @param val
	 */
	public synchronized void setBlocking(Boolean val) {
		blocking = val;
	}

	public synchronized void setDataHandler(DataHandler handler) {
		this._handler = handler;
	}
	
	public synchronized Session getSession() throws SessionDestroyedException, InterruptedException {
		if (blocking)
		{
			while (session == null)
			{
				if (executor.isShutdown()) throw new SessionDestroyedException();
				wait();
			}
			
			return session;
		}
		else
		{
			if (session == null) throw new SessionDestroyedException();
			return session;
		}
	}
	   
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			DTNClient.this.service = DTNService.Stub.asInterface(service);
			
			Session s = new Session(DTNClient.this.service, _packageName);
			executor.execute( new InitializeTask(s, _registration) );
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			_session_handler.onSessionDisconnected();
			DTNClient.this.service = null;
		}
	};
	
	private BroadcastReceiver _state_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.REGISTRATION))
			{
				Log.i(TAG, "registration successful");
				Session s = new Session(DTNClient.this.service, _packageName);
				executor.execute( new InitializeTask(s, _registration) );
			}
		}
	};
	
	private class InitializeTask implements Runnable {
		
		private Session session = null;
		private Registration reg = null;
		
		public InitializeTask(Session session, Registration reg)
		{
			this.reg = reg;
			this.session = session;
		}

		@Override
		public void run() {
			try {
				this.session.initialize();
				synchronized(DTNClient.this) {
					DTNClient.this.session = this.session;
					DTNClient.this.notifyAll();
				}
				_session_handler.onSessionConnected(this.session);
			} catch (Exception e) {
				register(reg);
			}
		}
	}
	
	private de.tubs.ibr.dtn.api.DTNSessionCallback mCallback = new de.tubs.ibr.dtn.api.DTNSessionCallback.Stub() {
		@Override
		public void startBundle(Bundle bundle) throws RemoteException {
			if (_handler == null) return;
			_handler.startBundle(bundle);
		}

		@Override
		public void endBundle() throws RemoteException {
			if (_handler == null) return;
			_handler.endBundle();
		}

		@Override
		public TransferMode startBlock(Block block) throws RemoteException {
			if (_handler == null) return TransferMode.NULL;
			return _handler.startBlock(block);
		}

		@Override
		public void endBlock() throws RemoteException {
			if (_handler == null) return;
			_handler.endBlock();
		}

		@Override
		public void characters(String data) throws RemoteException {
			if (_handler == null) return;
			_handler.characters(data);
		}

		@Override
		public ParcelFileDescriptor fd() throws RemoteException {
			if (_handler == null) return null;
			return _handler.fd();
		}

		@Override
		public void progress(long current, long length) throws RemoteException {
			if (_handler == null) return;
			_handler.progress(current, length);
		}

		@Override
		public void payload(byte[] data) throws RemoteException {
			if (_handler == null) return;
			_handler.payload(data);
		}
	};
	
	public class Session
	{
		private DTNService service = null;
		private DTNSession session = null;
		private String packageName = null;
		
		public Session(DTNService service, String packageName)
		{
			this.service = service;
			this.packageName = packageName;
		}
		
		public void initialize() throws RemoteException, SessionDestroyedException
		{
			// try to resume the previous session
			session = service.getSession(packageName);
			
			if (session == null)
			{
				Log.i(TAG, "Session not available, need to register!");
				throw new SessionDestroyedException();
			}
			
			Log.i(TAG, "session initialized");
		}
		
		public void destroy()
		{
			if (this.session == null) return;

			// send intent to destroy the session in the daemon
	    	Intent registrationIntent = new Intent(de.tubs.ibr.dtn.Intent.UNREGISTER);
	    	registrationIntent.putExtra("app", PendingIntent.getBroadcast(context, 0, new Intent(), 0)); // boilerplate
	    	context.startService(registrationIntent);
		}
		
		/**
		 * Send a string as a bundle to the given destination.
		 */
		public boolean send(EID destination, int lifetime, byte[] data) throws SessionDestroyedException
		{
			if (this.session == null) new SessionDestroyedException("session is null");
			
			try {
				if (destination instanceof GroupEndpoint)
				{
					// send the message to the daemon
					return session.sendGroup((GroupEndpoint)destination, lifetime, data);
				}

				// send the message to the daemon
				return session.send((SingletonEndpoint)destination, lifetime, data);
			} catch (RemoteException e) {
				 throw new SessionDestroyedException("send failed");
			}
		}
		
		/**
		 * Send the content of a file descriptor as bundle
		 */
		public boolean send(EID destination, int lifetime, ParcelFileDescriptor fd, Long length) throws SessionDestroyedException
		{
			if (this.session == null)  new SessionDestroyedException("session is null");
			
			try {
				if (destination instanceof GroupEndpoint)
				{
					// send the message to the daemon
					return session.sendGroupFileDescriptor((GroupEndpoint)destination, lifetime, fd, length);
				}
				else
				{
					// send the message to the daemon
					return session.sendFileDescriptor((SingletonEndpoint)destination, lifetime, fd, length);
				}
			} catch (RemoteException e) {
				return false;
			}
		}
		
		public Boolean queryNext() throws SessionDestroyedException
		{
			if (this.session == null) new SessionDestroyedException("session is null");
			
			try {
				return this.session.queryNext(mCallback);
			} catch (RemoteException e) {
				new SessionDestroyedException("remote session error");
			}
			return false;
		}
		
		public void delivered(BundleID id) throws SessionDestroyedException
		{
			if (this.session == null) new SessionDestroyedException("session is null");
			
			try {
				if (!this.session.delivered(id))
					new SessionDestroyedException("remote session error");
			} catch (RemoteException e) {
				new SessionDestroyedException("remote session error");
			}
		}
	};
	
	/**
	 * This method initialize the DTNClient connection to the DTN service. Call this method in the
	 * onCreate() method of your activity or service.
	 * @param context The current context.
	 * @param reg A registration object containing the application endpoint and all additional subscriptions.
	 * @throws ServiceNotAvailableException is thrown if the DTN service is not available on this device.
	 */
	public synchronized void initialize(Context context, Registration reg) throws ServiceNotAvailableException
	{
		// set the context
		this.context = context;
		
  		// get package name
  		_packageName = context.getApplicationContext().getPackageName();
		
  		// store registration
  		_registration = reg;
  		
    	Intent bindIntent = new Intent(DTNService.class.getName());
		List<ResolveInfo> list = context.getPackageManager().queryIntentServices(bindIntent, 0);    
		if (list.size() == 0) throw new ServiceNotAvailableException();		
  		
		// create new executor
		executor = Executors.newSingleThreadScheduledExecutor();
  		
		// register to daemon events
		IntentFilter rfilter = new IntentFilter(de.tubs.ibr.dtn.Intent.REGISTRATION);
		rfilter.addCategory(_packageName);
		context.registerReceiver(_state_receiver, rfilter );
  		
		// Establish a connection with the service.
		context.bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
	}
	
	/**
	 * This method terminates the DTNClient and shutdown all running background tasks. Call this
	 * method in the onDestroy() method of your activity or service.
	 */
	public synchronized void terminate()
	{	
		if (executor != null) {
			// shutdown the executor
			executor.shutdown();
			notifyAll();
			
			// unregister to daemon events
			context.unregisterReceiver(_state_receiver);
			
	        // Detach our existing connection.
			context.unbindService(mConnection);
			
			// wait until all jobs are finished
			try {
				executor.awaitTermination(2, TimeUnit.SECONDS);
			} catch (InterruptedException e) { }
		}
		
		Log.i(TAG, "BasicDTNClient terminated.");
	}
	
    public void register(Registration reg)
    {
    	Intent registrationIntent = new Intent(de.tubs.ibr.dtn.Intent.REGISTER);
    	registrationIntent.putExtra("app", PendingIntent.getBroadcast(context, 0, new Intent(), 0)); // boilerplate
    	registrationIntent.putExtra("registration", (Parcelable)reg);
    	context.startService(registrationIntent);
    }
    
    public void unregister()
    {
    	Intent registrationIntent = new Intent(de.tubs.ibr.dtn.Intent.UNREGISTER);
    	registrationIntent.putExtra("app", PendingIntent.getBroadcast(context, 0, new Intent(), 0)); // boilerplate
    	context.startService(registrationIntent);
    }
	
	public DTNService getDTNService()
	{
		return service;
	}
}
