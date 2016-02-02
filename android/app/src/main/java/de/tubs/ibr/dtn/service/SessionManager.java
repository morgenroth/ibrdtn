/*
 * SessionManager.java
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

import java.util.HashMap;
import java.util.List;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.util.Log;
import de.tubs.ibr.dtn.api.GroupEndpoint;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.service.db.ApiDatabase;
import de.tubs.ibr.dtn.service.db.Endpoint;
import de.tubs.ibr.dtn.service.db.Session;
import de.tubs.ibr.dtn.swig.NativeSessionException;

public class SessionManager {
	
	private final String TAG = "SessionManager";
	
	private Context mContext = null;
	private final ApiDatabase mDatabase = new ApiDatabase();
	
	private HashMap<Session, ClientSession> mSessions = new HashMap<Session, ClientSession>();
	
	public SessionManager(Context context)
	{
		mContext = context;
	}
	
	public synchronized void initialize()
	{
		// open database
		mDatabase.open(mContext);
		
		// daemon goes up, create all sessions
		List<Session> sessions = mDatabase.getSessions();
		
		for (Session s : sessions) {
			// check if the application is still installed
			if (isPackageInstalled(s.getPackageName())) {
				// restore the session instance
				ClientSession client = new ClientSession(this, s);
				
				// restore session registration
				restore(s, client);
				
				// put the client session into the client list
				mSessions.put(s, client);
			} else {
				Log.i(TAG, "Application " + s.getPackageName() + " is no longer installed.");
				
				// delete session if app is not installed
				mDatabase.removeSession(s);
			}
		}
	}
	
	public synchronized void destroy()
	{
		// daemon goes down, destroy all sessions
		for (ClientSession s : mSessions.values()) {
			s.destroy();
		}
		
		mSessions.clear();
		
		// close the database
		mDatabase.close();
	}
	
	private void restore(Session s, ClientSession client) {
		// restore session endpoints
		List<Endpoint> endpoints = mDatabase.getEndpoints(s);
		
		for (Endpoint e : endpoints) {
			try {
				client.addEndpoint(e);
			} catch (NativeSessionException e1) {
				Log.e(TAG, "can not restore endpoint registration " + e, e1);
			}
		}
	}
	
	private void apply(Session s, ClientSession client, Registration reg) throws NativeSessionException {
	    if (reg.getEndpoint() != null) {
    		// set default session endpoint
    		mDatabase.setDefaultEndpoint(s, reg.getEndpoint());
    		client.setDefaultEndpoint(reg.getEndpoint());
	    }
		
		// iterate through new endpoints
		for (GroupEndpoint group : reg.getGroups()) {
			if (mDatabase.createEndpoint(s, group) != null) {
				client.addEndpoint(group);
			}
		}
		
		// get already registered endpoints
		List<Endpoint> endpoints = mDatabase.getEndpoints(s);
		
		// iterate through endpoints
		for (Endpoint e : endpoints) {
			// check if the endpoint is still registered
			boolean active = false;
			
			// check if registered as group
			for (GroupEndpoint group : reg.getGroups()) {
				if (e.equals(group)) {
					// mark as active
					active = true;
					break;
				}
			}
			
			if (!active) {
				// remove registered endpoint
				mDatabase.removeEndpoint(e);
				
				// remove endpoint from active client
				client.removeEndpoint(e);
			}
		}
	}

	@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
	public synchronized void register(String packageName, Registration reg)
	{
		// get session object
		Session s = mDatabase.getSession(packageName);
		
		try {
			if (s == null)
			{
				// create a new registration
				s = mDatabase.createSession(packageName, reg.getEndpoint());
				
				// restore the session instance
				ClientSession client = new ClientSession(this, s);
				
				// apply registration to the session
				apply(s, client, reg);
				
				// put the new client session into the client list
				mSessions.put(s, client);
			}
			else
			{
				// get existing session object
				ClientSession client = mSessions.get(s);
				
				// update registration
				apply(s, client, reg);
			}
			
			// send out registration intent to the application with the existing session key
			Intent broadcastIntent = new Intent(de.tubs.ibr.dtn.Intent.REGISTRATION);
			broadcastIntent.addCategory(s.getPackageName());
			broadcastIntent.setPackage(s.getPackageName());
			broadcastIntent.putExtra("key", s.getSessionKey());
			
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR1) {
				broadcastIntent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
			}
	
			// send notification intent
			mContext.sendBroadcast(broadcastIntent, de.tubs.ibr.dtn.Intent.PERMISSION_COMMUNICATION);
		} catch (NativeSessionException e) {
			Log.e(TAG, "failure while registering a session", e);
		}
	}
	
	public synchronized void unregister(String packageName)
	{
		Session s = mDatabase.getSession(packageName);
		
		// silently return if the session does not exists
		if (s == null) return;
		
		Log.i(TAG, "destroy session " + s);
		
		// destroy active session
		mSessions.get(s).destroy();
		
		// remove session from active session list
		mSessions.remove(s);
		
		// delete session from database
		mDatabase.removeSession(s);
	}
	
	public synchronized void join(ClientSession client, Session s, GroupEndpoint group) throws NativeSessionException {
        // add new endpoint
        if (mDatabase.createEndpoint(s, group) != null) {
            client.addEndpoint(group);
        }
	}
	
    public synchronized void leave(ClientSession client, Session s, GroupEndpoint group) throws NativeSessionException {
        // get already registered endpoints
        List<Endpoint> endpoints = mDatabase.getEndpoints(s);
        
        // iterate through endpoints
        for (Endpoint e : endpoints) {
            // check if registered as group
            if (e.equals(group)) {
                // remove registered endpoint
                mDatabase.removeEndpoint(e);
                
                // remove endpoint from active client
                client.removeEndpoint(e);
                
                break;
            }
        }
    }
    
    public synchronized List<Endpoint> getEndpoints(Session s) throws NativeSessionException {
        return mDatabase.getEndpoints(s);
    }
	
	public synchronized ClientSession getSession(String[] packageNames, String sessionKey)
	{
		for (String packageName : packageNames) {
			Session s = mDatabase.getSession(packageName, sessionKey);
			if (s != null) {
				// access granted
				return mSessions.get(s);
			}
		}

		// access denied
		return null;
	}
	
	public Context getContext() {
		return mContext;
	}
	
	private boolean isPackageInstalled(String packagename) {
		PackageManager pm = mContext.getPackageManager();
		try {
			pm.getPackageInfo(packagename, PackageManager.GET_ACTIVITIES);
			return true;
		} catch (NameNotFoundException e) {
			return false;
		}
	}
}
