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

import android.content.Context;
import android.content.Intent;
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
			// restore the session instance
			ClientSession client = new ClientSession(this, s);
			
			// restore session registration
			restore(s, client);
			
			// put the client session into the client list
			mSessions.put(s, client);
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
	
	private void apply(Session s, ClientSession client, Registration reg) {
		// check default session endpoint
		if (reg.getEndpoint() != s.getDefaultEndpoint()) {
			if (reg.getEndpoint() != null) {
				s.setDefaultEndpoint(reg.getEndpoint());
			}
		}
		
		// get already registered endpoints
		List<Endpoint> endpoints = mDatabase.getEndpoints(s);
		
		// iterate through new endpoints
		for (GroupEndpoint group : reg.getGroups()) {
			// check if this group is registered
			for (Endpoint e : endpoints) {
				if (e.equals(group)) {
					// TODO: ...
				}
			}
		}
		
		// TODO: complete this...
	}

	public synchronized void register(String packageName, Registration reg)
	{
		// get session object
		Session s = mDatabase.getSessionByPackageName(packageName);
		
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
        broadcastIntent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
        broadcastIntent.putExtra("key", s.getSessionKey());

        // send notification intent
        mContext.sendBroadcast(broadcastIntent);
	}
	
	public synchronized void unregister(String packageName)
	{
		Session s = mDatabase.getSessionByPackageName(packageName);
		
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
	
	public synchronized ClientSession getSession(String[] packageNames, String sessionKey)
	{
		Session s = mDatabase.getSession(sessionKey);
		
		if (checkPermissions(packageNames, s)) {
			// access granted
			return mSessions.get(s);
		} else {
			// access denied
			return null;
		}
	}
	
	private boolean checkPermissions(String[] packageNames, Session session)
	{
		// return false if the requested session does not exist
		if (session == null) return false;
		
		// check if session is allowed to access by one of the package names
		for (String packageName : packageNames) {
			if (packageName.equals(session.getPackageName())) {
				if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "access to session " + session + " granted");
				return true;
			}
		}
		
		if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "access to session " + session + " denied");
		
		return false;
	}
	
	public Context getContext() {
		return mContext;
	}
}
