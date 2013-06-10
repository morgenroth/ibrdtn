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

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.Parcelable;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.util.Log;
import de.tubs.ibr.dtn.DTNService;

public final class DTNClient {
	
	private static final String TAG = "DTNClient";
	
	private static final String PREF_SESSION_KEY = "dtn_session_key";

    // DTN service provided by IBR-DTN
    private DTNService mService = null;
    
    // session object
	private Session mSession = null;
	
	// data handler which processes incoming bundles
	private DataHandler mHandler = null;
	
	private Context mContext = null;
	
	private Registration mRegistration = null;
	
    private Boolean mBlocking = true;
    
    private SessionConnection mSessionHandler = null;
    
    private Boolean mShutdown = false;
    
    public DTNClient(SessionConnection handler) {
    	mSessionHandler = handler;
    }
    
    public DTNClient() {
    	// add dummy handler
    	mSessionHandler = new SessionConnection() {
			public void onSessionConnected(Session session) { }

			public void onSessionDisconnected() { }
    	};
    }
	
	/**
	 * If blocking is enabled, the getSession() method will block until a valid session
	 * exists. If set to false, the method would return null if there is no active session.
	 * @param val
	 */
	public synchronized void setBlocking(Boolean val) {
		mBlocking = val;
	}

	public synchronized void setDataHandler(DataHandler handler) {
		mHandler = handler;
	}
	
	public synchronized Session getSession() throws SessionDestroyedException, InterruptedException {
		if (mBlocking)
		{
			while (mSession == null)
			{
				if (mShutdown) throw new SessionDestroyedException();
				wait();
			}
			
			return mSession;
		}
		else
		{
			if ((mSession == null) || mShutdown) throw new SessionDestroyedException();
			return mSession;
		}
	}
	   
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = DTNService.Stub.asInterface(service);
			initializeSession(mRegistration);
		}

		public void onServiceDisconnected(ComponentName name) {
			mSessionHandler.onSessionDisconnected();
			mService = null;
		}
	};
	
	private BroadcastReceiver mStateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.REGISTRATION))
			{
				Log.i(TAG, "registration successful");
				
				// store session key
				String session_key = intent.getStringExtra("key");
				
				SharedPreferences pm = PreferenceManager.getDefaultSharedPreferences(mContext);
				pm.edit().putString(PREF_SESSION_KEY, session_key).commit();
				
				// initialize the session
				initializeSession(mRegistration);
			}
		}
	};
	
	private void initializeSession(Registration reg) {
		try {
			// do not initialize if the service is not connected
			if (mService == null) return;
			
			Session s = new Session(mContext, mService, mCallback);
			
			s.initialize();
			
			synchronized(DTNClient.this) {
				mSession = s;
				notifyAll();
			}
			
			mSessionHandler.onSessionConnected(s);
		} catch (Exception e) {
	    	Intent registrationIntent = new Intent(de.tubs.ibr.dtn.Intent.REGISTER);
	    	registrationIntent.putExtra("app", PendingIntent.getBroadcast(mContext, 0, new Intent(), 0)); // boilerplate
	    	registrationIntent.putExtra("registration", (Parcelable)reg);
	    	mContext.startService(registrationIntent);
		}
	}
	
	private de.tubs.ibr.dtn.api.DTNSessionCallback mCallback = new de.tubs.ibr.dtn.api.DTNSessionCallback.Stub() {
		public void startBundle(Bundle bundle) throws RemoteException {
			if (mHandler == null) return;
			mHandler.startBundle(bundle);
		}

		public void endBundle() throws RemoteException {
			if (mHandler == null) return;
			mHandler.endBundle();
		}

		public TransferMode startBlock(Block block) throws RemoteException {
			if (mHandler == null) return TransferMode.NULL;
			return mHandler.startBlock(block);
		}

		public void endBlock() throws RemoteException {
			if (mHandler == null) return;
			mHandler.endBlock();
		}

		public ParcelFileDescriptor fd() throws RemoteException {
			if (mHandler == null) return null;
			return mHandler.fd();
		}

		public void progress(long current, long length) throws RemoteException {
			if (mHandler == null) return;
			mHandler.progress(current, length);
		}

		public void payload(byte[] data) throws RemoteException {
			if (mHandler == null) return;
			mHandler.payload(data);
		}
	};
	
	public static class Session
	{
		private Context mContext = null;
		private DTNService mService = null;
		private DTNSession mSession = null;
		private DTNSessionCallback mCallback = null;
		
		public Session(Context context, DTNService service, DTNSessionCallback callback) {
			mContext = context;
			mService = service;
			mCallback = callback;
		}
		
		public void initialize() throws RemoteException, SessionDestroyedException {
			SharedPreferences pm = PreferenceManager.getDefaultSharedPreferences(mContext);
			if (!pm.contains(PREF_SESSION_KEY)) {
				Log.i(TAG, "No session key available, need to register!");
				throw new SessionDestroyedException();
			}
			
			// try to resume the previous session
			mSession = mService.getSession(pm.getString(PREF_SESSION_KEY, ""));
			
			if (mSession == null)
			{
				Log.i(TAG, "Session not available, need to register!");
				throw new SessionDestroyedException();
			}
			
			Log.i(TAG, "session initialized");
		}
		
		public void destroy() {
			if (mSession == null) return;

			// send intent to destroy the session in the daemon
	    	Intent registrationIntent = new Intent(de.tubs.ibr.dtn.Intent.UNREGISTER);
	    	registrationIntent.putExtra("app", PendingIntent.getBroadcast(mContext, 0, new Intent(), 0)); // boilerplate
	    	mContext.startService(registrationIntent);
		}
		
		/**
		 * Send a string as a bundle to the given destination.
		 */
		public BundleID send(Bundle bundle, byte[] data) throws SessionDestroyedException {
			if (mSession == null) new SessionDestroyedException("session is null");
			
			try {
				// send the message to the daemon
				return mSession.send(null, bundle, data);
			} catch (RemoteException e) {
				 throw new SessionDestroyedException("send failed");
			}
		}
		
		/**
		 * Send a string as a bundle to the given destination.
		 */
		public BundleID send(EID destination, long lifetime, byte[] data) throws SessionDestroyedException {
			Bundle bundle = new Bundle();
			bundle.setDestination(destination);
			bundle.setLifetime(lifetime);
			
			return send(bundle, data);
		}
		
		/**
		 * Send the content of a file descriptor as bundle
		 */
		public BundleID send(Bundle bundle, ParcelFileDescriptor fd) throws SessionDestroyedException {
			if (mSession == null)  new SessionDestroyedException("session is null");
			
			try {
				// send the message to the daemon
				return mSession.sendFileDescriptor(null, bundle, fd);
			} catch (RemoteException e) {
				return null;
			}
		}
		
		/**
		 * Send the content of a file descriptor as bundle
		 */
		public BundleID send(EID destination, long lifetime, ParcelFileDescriptor fd) throws SessionDestroyedException {
			Bundle bundle = new Bundle();
			bundle.setDestination(destination);
			bundle.setLifetime(lifetime);
			
			return send(bundle, fd);
		}
		
		public Boolean queryNext() throws SessionDestroyedException {
			if (mSession == null) new SessionDestroyedException("session is null");
			
			try {
				return mSession.queryNext(mCallback);
			} catch (RemoteException e) {
				new SessionDestroyedException("remote session error");
			}
			return false;
		}
		
		public Boolean query(BundleID id) throws SessionDestroyedException {
		    if (mSession == null) new SessionDestroyedException("session is null");
		    
            try {
                return mSession.query(mCallback, id);
            } catch (RemoteException e) {
                new SessionDestroyedException("remote session error");
            }
            return false;
		}
		
        public Boolean queryInfoNext() throws SessionDestroyedException {
            if (mSession == null) new SessionDestroyedException("session is null");
            
            try {
                return mSession.queryInfoNext(mCallback);
            } catch (RemoteException e) {
                new SessionDestroyedException("remote session error");
            }
            return false;
        }
        
        public Boolean queryInfo(BundleID id) throws SessionDestroyedException {
            if (mSession == null) new SessionDestroyedException("session is null");
            
            try {
                return mSession.queryInfo(mCallback, id);
            } catch (RemoteException e) {
                new SessionDestroyedException("remote session error");
            }
            return false;
        }
		
		public void delivered(BundleID id) throws SessionDestroyedException {
			if (mSession == null) new SessionDestroyedException("session is null");
			
			try {
				if (!mSession.delivered(id))
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
	public synchronized void initialize(Context context, Registration reg) throws ServiceNotAvailableException {
		// set the context
		mContext = context;
		
  		// store registration
  		mRegistration = reg;
  		
    	Intent bindIntent = new Intent(DTNService.class.getName());
		List<ResolveInfo> list = context.getPackageManager().queryIntentServices(bindIntent, 0);    
		if (list.size() == 0) throw new ServiceNotAvailableException();		
  		
		// create new executor
		mShutdown = false;
  		
		// register to daemon events
		IntentFilter rfilter = new IntentFilter(de.tubs.ibr.dtn.Intent.REGISTRATION);
		rfilter.addCategory(context.getApplicationContext().getPackageName());
		context.registerReceiver(mStateReceiver, rfilter );
  		
		// Establish a connection with the service.
		context.bindService(bindIntent, mConnection, Context.BIND_AUTO_CREATE);
	}
	
	/**
	 * This method terminates the DTNClient and shutdown all running background tasks. Call this
	 * method in the onDestroy() method of your activity or service.
	 */
	public synchronized void terminate() {
		mShutdown = true;
		notifyAll();
		
		// unregister to daemon events
		mContext.unregisterReceiver(mStateReceiver);
		
        // Detach our existing connection.
		mContext.unbindService(mConnection);
	}
	
	public DTNService getDTNService() {
		return mService;
	}
}
