package de.tubs.ibr.dtn.service;

import ibrdtn.api.Base64;

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
import de.tubs.ibr.dtn.api.Registration;
import android.util.Base64InputStream;
import android.util.Log;

public class SessionManager {
	
	private final String TAG = "SessionManager";
	private Boolean _state = false;
	
	private HashMap<String, ClientSession> _sessions = new HashMap<String, ClientSession>();
	private HashMap<String, Registration> _registrations = new HashMap<String, Registration>();
	
	public SessionManager()
	{
	}
	
	public synchronized void initialize(Context context)
	{
		// daemon goes up, create all sessions
		for (Entry<String, Registration> entry : _registrations.entrySet())
		{
			// create a new session
			ClientSession session = new ClientSession(context, entry.getValue(), entry.getKey());
			_sessions.put(entry.getKey(), session);
			session.initialize();
		}
		_state = true;
	}
	
	public synchronized void terminate()
	{
		// daemon goes down, destroy all sessions
		for (ClientSession session : _sessions.values())
		{
			session.terminate();
		}
		
		_sessions.clear();
		_state = false;
	}
	
	public void restoreRegistrations(Context context)
	{
		SharedPreferences prefs = context.getSharedPreferences("registrations", Context.MODE_PRIVATE);
		Map<String, ?> m = prefs.getAll();
		for (Entry<String, ?> entry : m.entrySet())
		{
			String data = (String)entry.getValue();
			
			try {
				ObjectInputStream ois = new ObjectInputStream(new Base64InputStream( new ByteArrayInputStream(data.getBytes()), android.util.Base64.DEFAULT ));
				Object regobj = ois.readObject();
				
				if (regobj instanceof Registration) {
					// re-register registration
					Registration reg = (Registration)regobj;
					register(context, entry.getKey(), reg);
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
	
	public void saveRegistrations(Context context)
	{
		SharedPreferences prefs = context.getSharedPreferences("registrations", Context.MODE_PRIVATE);
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
	
	public void saveRegistration(Context context, String packageName, Registration reg)
	{
		SharedPreferences prefs = context.getSharedPreferences("registrations", Context.MODE_PRIVATE);
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
	
	public synchronized void register(Context context, String packageName, Registration reg)
	{
    	// register an application
    	// - add it to the list of applications
    	// - if the daemon is running, create a session
		Log.i(TAG, "register application: " + packageName + "; " + reg.getEndpoint());
		
		// abort if the registration is already known
		if (_registrations.containsKey(packageName))
		{
			Registration previous_reg = _registrations.get(packageName);
			
			// abort if the registration has not been changed
			if (previous_reg.equals(reg)) return;
			
			Log.d(TAG, "terminate and remove old registration");
			
			// remove previous registation
			_registrations.remove(packageName);
			
			// add registration to the hashmap
			_registrations.put(packageName, reg);
			
			// terminate the session if the daemon is running
			if (_state)
			{
				// daemon is up
				// destroy the session
				_sessions.get(packageName).terminate();
				_sessions.remove(packageName);
			}
		}
		else
		{
			// add registration to the hashmap
			_registrations.put(packageName, reg);
		}

		// save registration in the preferences
		saveRegistration(context, packageName, reg);
		
		if (_state)
		{
			// daemon is up
			// create a new session
			ClientSession session = new ClientSession(context, _registrations.get(packageName), packageName);
			_sessions.put(packageName, session);
			session.initialize();
		}
	}
	
	public synchronized void unregister(Context context, String packageName)
	{
    	// unregister an application
    	// - remove it from the list of application
    	// - destroy the corresponding session, if exists
		Log.i(TAG, "unregister application: " + packageName);
		
		// stop here if there is no registration
		if (!_registrations.containsKey(packageName)) return;
		
		_registrations.remove(packageName);

		if (_state)
		{
			// daemon is up
			// destroy the session
			_sessions.get(packageName).terminate();
			_sessions.remove(packageName);
		}
		
		// save current registration state
		saveRegistrations(context);
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
	
	public synchronized void destroySessions()
	{
		for (ClientSession cs : _sessions.values())
		{
			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "Destroy session: " + cs.getPackageName());
			cs.terminate();
		}
		
		_sessions.clear();
	}
}
