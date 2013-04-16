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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OptionalDataException;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.util.Base64InputStream;
import android.util.Log;
import de.tubs.ibr.dtn.api.Registration;
import de.tubs.ibr.dtn.util.Base64;

public class SessionManager {
	
	private final String TAG = "SessionManager";
	
	private Context _context = null;
	
	private HashMap<String, ClientSession> _sessions = new HashMap<String, ClientSession>();
	private HashMap<String, Registration> _registrations = new HashMap<String, Registration>();
	
	public SessionManager(Context context)
	{
		this._context = context;
	}
	
	public synchronized void initialize()
	{
		// daemon goes up, create all sessions
		for (Entry<String, Registration> entry : _registrations.entrySet())
		{
			// create a new session
			ClientSession session = new ClientSession(this._context, entry.getValue(), entry.getKey());
			_sessions.put(entry.getKey(), session);
		}
	}
	
	public synchronized void destroy()
	{
        // save registrations
        saveRegistrations();
        
        // daemon goes down, destroy all sessions
        for (ClientSession s : _sessions.values()) {
            s.destroy();
        }
        
		_sessions.clear();
	}
	
	public void restoreRegistrations()
	{
		SharedPreferences prefs = this._context.getSharedPreferences("registrations", Context.MODE_PRIVATE);
		Map<String, ?> m = prefs.getAll();
		for (Entry<String, ?> entry : m.entrySet())
		{
			String data = (String)entry.getValue();
			
			try {
				ObjectInputStream ois = new ObjectInputStream(new Base64InputStream( new ByteArrayInputStream(data.getBytes()), android.util.Base64.DEFAULT ));
				Object regobj = ois.readObject();
				ois.close();
				
				if (regobj instanceof Registration) {
					// re-register registration
					Registration reg = (Registration)regobj;
					register(entry.getKey(), reg);
					Log.d(TAG, "registration restored for " + entry.getKey());
				} else {
					// delete registration
					Editor ed = prefs.edit();
					ed.remove(entry.getKey());
					ed.commit();
				}
			} catch (OptionalDataException e) {
				Log.e(TAG, "error on restore registrations", e);
			} catch (ClassNotFoundException e) {
				Log.e(TAG, "error on restore registrations", e);
			} catch (IOException e) {
				Log.e(TAG, "error on restore registrations", e);
			} catch (java.lang.IllegalArgumentException e) {
				Log.e(TAG, "error on restore registrations", e);
				
				// destroy all registrations
				Editor ed = prefs.edit();
				ed.clear();
				ed.commit();
			}
		}
	}
	
	private void saveRegistrations()
	{
		SharedPreferences prefs = this._context.getSharedPreferences("registrations", Context.MODE_PRIVATE);
		Editor edit = prefs.edit();
		edit.clear();
		
		for (Entry<String, Registration> entry : _registrations.entrySet())
		{
			try {
				ByteArrayOutputStream baos = new ByteArrayOutputStream();
				ObjectOutputStream oos = new ObjectOutputStream(baos);
				oos.writeObject(entry.getValue());
				edit.putString(entry.getKey(), Base64.encodeBytes( baos.toByteArray() ) );
				Log.d(TAG, "registration saved for " + entry.getKey());
			} catch (IOException e) {
				Log.e(TAG, "error on save registrations", e);
			}
		}
		
		edit.commit();
	}
	
	public void saveRegistration(String packageName, Registration reg)
	{
		SharedPreferences prefs = this._context.getSharedPreferences("registrations", Context.MODE_PRIVATE);
		Editor edit = prefs.edit();

		try {
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			ObjectOutputStream oos = new ObjectOutputStream(baos);
			oos.writeObject(reg);
			edit.putString(packageName, Base64.encodeBytes( baos.toByteArray() ) );
			Log.d(TAG, "registration saved for " + packageName);
		} catch (IOException e) {
			Log.e(TAG, "error on save registration", e);
		}

		edit.commit();
	}
	
	public synchronized void register(String packageName, Registration reg)
	{
    	// register an application
    	// - add it to the list of applications
    	// - if the daemon is running, create a session
		Log.i(TAG, "register application: " + packageName + "; " + reg.getEndpoint());
		
		// abort if the registration is already known
		if (_registrations.containsKey(packageName))
		{
            Registration previous_reg = _registrations.get(packageName);
            
            if (previous_reg != null) {
                // abort if the registration has not been changed
                if (previous_reg.equals(reg)) return;
            } 
		        
			// update the registration
            // remove previous registation
            _registrations.remove(packageName);
            
            // add registration to the hashmap
            _registrations.put(packageName, reg);
		    
            // get the existing session
            _sessions.get(packageName).update(reg);
		}
		else
		{
			// add registration to the hashmap
			_registrations.put(packageName, reg);

            // create a new session
            ClientSession session = new ClientSession(this._context, reg, packageName);
            _sessions.put(packageName, session);
		}

		// save registration in the preferences
		saveRegistration(packageName, reg);
	}
	
	public synchronized void unregister(String packageName)
	{
    	// unregister an application
    	// - remove it from the list of application
    	// - destroy the corresponding session, if exists
		Log.i(TAG, "unregister application: " + packageName);
		
		// stop here if there is no registration
		if (!_registrations.containsKey(packageName)) return;
		
		_registrations.remove(packageName);

		// daemon is up
		// destroy the session
		_sessions.get(packageName).destroy();
		_sessions.remove(packageName);
		
		// save current registration state
		saveRegistrations();
	}
	
	public synchronized ClientSession getSession(String packageName)
	{
		ClientSession s = _sessions.get(packageName);
		
		if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "returned session: " + s.getPackageName());
		
		return s;
	}
	
	public synchronized void removeSession(String packageName)
	{
		_sessions.remove(packageName);
	}
}
