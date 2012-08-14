/*
 * Preferences.java
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
package de.tubs.ibr.dtn.daemon;

import java.io.IOException;
import java.net.NetworkInterface;
import java.util.Enumeration;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.support.v4.view.MenuItemCompat;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonManager;
import de.tubs.ibr.dtn.service.DaemonProcess;
import de.tubs.ibr.dtn.service.DaemonService;

public class Preferences extends PreferenceActivity {
	
	private final String TAG = "Preferences";
	
	private DTNService service = null;
	
	// progress dialog for the send process
	private ProgressDialog pd = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			Preferences.this.service = DTNService.Stub.asInterface(service);
			Log.i(TAG, "service connected");
			
			// on first startup ask for permissions to collect statistical data
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
			if (!prefs.contains("collect_stats")) {
				showStatisticLoggerDialog(Preferences.this);
			}
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			service = null;
		}
	};
	
	public static void showStatisticLoggerDialog(final Activity activity) {
		DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
		    @Override
		    public void onClick(DialogInterface dialog, int which) {
		    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(activity);
		    	
		        switch (which){
		        case DialogInterface.BUTTON_POSITIVE:
		        	prefs.edit().putBoolean("collect_stats", true).commit();
		            break;

		        case DialogInterface.BUTTON_NEGATIVE:
		        	prefs.edit().putBoolean("collect_stats", false).commit();
		            break;
		        }
		        
		        activity.finish();
		        activity.startActivity(new Intent(activity, Preferences.class));
		    }
		};

		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setTitle(R.string.alert_statistic_logger_title);
		builder.setMessage(activity.getResources().getString(R.string.alert_statistic_logger_dialog));
		builder.setPositiveButton(activity.getResources().getString(android.R.string.yes), dialogClickListener);
		builder.setNegativeButton(activity.getResources().getString(android.R.string.no), dialogClickListener);
		builder.show();
	}
	
	private BroadcastReceiver _state_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.STATE))
			{
				@SuppressWarnings("deprecation")
				CheckBoxPreference enabledSwitch = (CheckBoxPreference) findPreference("enabledSwitch");
				
				String state = intent.getStringExtra("state");
				DaemonState ds = DaemonState.valueOf(state);
				switch (ds)
				{
				case ONLINE:
					enabledSwitch.setChecked(true);
					break;
					
				case OFFLINE:
					enabledSwitch.setChecked(false);
					break;
					
				case ERROR:
					enabledSwitch.setChecked(false);
					break;
					
				default:
					break;
				}
				
				enabledSwitch.setEnabled(true);
			}
		}
	};
	
	private class ModifyCloudUplink extends AsyncTask<MenuItem, Integer, MenuItem> {
		protected MenuItem doInBackground(MenuItem... args)
		{
			try {
		    	if (args[0].isChecked()) {
		    		if (service.isRunning())
		    			DaemonManager.getInstance().disableCloudUplink();
		    	} else {
		    		if (service.isRunning())
		    			DaemonManager.getInstance().enableCloudUplink();
		    	}
		    	
				return args[0];
			} catch (RemoteException e) {
				return null;
			}
		}

		protected void onProgressUpdate(Integer... progress) {
		}

		protected void onPostExecute(MenuItem result)
		{
			if (result == null) {
				pd.cancel();
			} else {
		    	Boolean onoff = !result.isChecked();
		    	
		    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
		    	Editor edit = prefs.edit();
		    	edit.putBoolean("cloud_uplink", onoff);
		    	edit.apply();
		    	
		    	result.setChecked(onoff);
		    	result.setIcon(onoff ? R.drawable.ic_cloud_off : R.drawable.ic_cloud);
		    	result.setTitle(onoff ? R.string.clouduplink_off : R.string.clouduplink_on);
		    	
				pd.dismiss();
			}
		}
	}
	
	private class ClearStorageTask extends AsyncTask<String, Integer, Boolean> {
		protected Boolean doInBackground(String... files)
		{
			try {
		    	if (service.isRunning())
		    	{
		    		return false;
		    	}
		    	service.clearStorage();
				return true;
			} catch (RemoteException e) {
				return false;
			}
		}

		protected void onProgressUpdate(Integer... progress) {
		}

		protected void onPostExecute(Boolean result)
		{
			if (result)
			{
				pd.dismiss();
			}
			else
			{
				pd.cancel();
	    		Toast toast = Toast.makeText(Preferences.this, "Daemon is running! Please stop the daemon first.", Toast.LENGTH_LONG);
	    		toast.show();
			}
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.main_menu, menu);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemCloudUplink), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemNeighbors), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemShowLog), MenuItemCompat.SHOW_AS_ACTION_IF_ROOM | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    MenuItemCompat.setShowAsAction(menu.findItem(R.id.itemClearStorage), MenuItemCompat.SHOW_AS_ACTION_NEVER | MenuItemCompat.SHOW_AS_ACTION_WITH_TEXT);
	    return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		MenuItem cloud = menu.findItem(R.id.itemCloudUplink);
		
    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
    	Boolean onoff = prefs.getBoolean("cloud_uplink", false);
		
		cloud.setChecked(onoff);
		cloud.setIcon(onoff ? R.drawable.ic_cloud_off : R.drawable.ic_cloud);
		cloud.setTitle(onoff ? R.string.clouduplink_off : R.string.clouduplink_on);
		
		return super.onPrepareOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {        
	    case R.id.itemShowLog:
	    {
			Intent i = new Intent(Preferences.this, LogActivity.class);
			startActivity(i);
	    	return true;
	    }
	    
	    case R.id.itemClearStorage:
	    {
			pd = ProgressDialog.show(Preferences.this, getResources().getString(R.string.wait), getResources().getString(R.string.clearingstorage), true, false);
			(new ClearStorageTask()).execute();
	    	return true;
	    }
	    
	    case R.id.itemCloudUplink:
	    {
	    	pd = ProgressDialog.show(Preferences.this, getResources().getString(R.string.wait), getResources().getString(R.string.alteratecloudstate), true, false);
	    	(new ModifyCloudUplink()).execute(item);
	    	return true;
	    }
	        
	    case R.id.itemNeighbors:
	    {
	    	// open neighbor list activity
	    	Intent i = new Intent(Preferences.this, NeighborList.class);
	    	startActivity(i);
	    	return true;
	    }
	    
	    default:
	        return super.onOptionsItemSelected(item);
	    }
	}
	
	public static void initializeDefaultPreferences(Context context) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		
		if (!prefs.contains("endpoint_id")) {
			Editor e = prefs.edit();
			e.putString("endpoint_id", DaemonProcess.getUniqueEndpointID(context).toString());
			
			try {
				// scan for known network devices
				for(Enumeration<NetworkInterface> list = NetworkInterface.getNetworkInterfaces(); list.hasMoreElements();)
			    {
		            NetworkInterface i = list.nextElement();
		            String iface = i.getDisplayName();
		            
		            if (	iface.contains("wlan") ||
		            		iface.contains("wifi") ||
		            		iface.contains("eth")
		            	) {
		            	e.putBoolean("interface_" + iface, true);
		            }
			    }
			} catch (IOException ex) { }
			
			e.commit();
		}
	}
	
	@SuppressWarnings("deprecation")
	@Override
	public void onCreate(Bundle savedInstanceState) {
		// initialize default values if configured set already
		initializeDefaultPreferences(this);
		
	    super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
		
		// disable daemon controls
		Preference enabledSwitch = (Preference) findPreference("enabledSwitch");
		enabledSwitch.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			@Override
			public boolean onPreferenceClick(Preference p) {
				p.setEnabled(false);
				
				if (((CheckBoxPreference) p).isChecked()) {
					// startup the daemon process
					final Intent intent = new Intent(Preferences.this, DaemonService.class);
					intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STARTUP);
					startService(intent);
				}
				else
				{
					// shutdown the daemon
					final Intent intent = new Intent(Preferences.this, DaemonService.class);
					intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_SHUTDOWN);
					startService(intent);
				}
				
				return true;
			}
		});
		
		try {
			PreferenceCategory pc = (PreferenceCategory) findPreference("interfaces");
			
			for(Enumeration<NetworkInterface> list = NetworkInterface.getNetworkInterfaces(); list.hasMoreElements();)
		    {
	            NetworkInterface i = list.nextElement();
	            String iface = i.getDisplayName();
	            CheckBoxPreference cb_i = new CheckBoxPreference(this);
	            
	            cb_i.setTitle(iface);
	            
	            if (i.isPointToPoint())
	            {
	            	cb_i.setSummary("Point-to-Point");
	            }
	            else if (i.isLoopback())
	            {
	            	
	            	cb_i.setSummary("Loopback");
	            }
	            
	            cb_i.setKey("interface_" + iface);
	            pc.addPreference(cb_i);
	            cb_i.setDependency(pc.getDependency());
		    }
		} catch (IOException e) { }
		
		// version information
		Preference version = findPreference("system_version");
		try {
			PackageInfo info = getPackageManager().getPackageInfo(getPackageName(), 0);
			version.setSummary(info.versionCode + "-" + info.versionName);
		} catch (NameNotFoundException e) { };
	}
	
    @Override
	protected void onPause() {
		unregisterReceiver(_state_receiver);
		
        // Detach our existing connection.
		unbindService(mConnection);
		
		super.onPause();
	}

	@Override
	protected void onResume() {
		IntentFilter ifilter = new IntentFilter(de.tubs.ibr.dtn.Intent.STATE);
		ifilter.addCategory(Intent.CATEGORY_DEFAULT);
  		registerReceiver(_state_receiver, ifilter );
  		
		// Establish a connection with the service.  We use an explicit
		// class name because we want a specific service implementation that
		// we know will be running in our own process (and thus won't be
		// supporting component replacement by other applications).
		bindService(new Intent(Preferences.this, 
				DaemonService.class), mConnection, Context.BIND_AUTO_CREATE);
  		
		super.onResume();
	}
}
