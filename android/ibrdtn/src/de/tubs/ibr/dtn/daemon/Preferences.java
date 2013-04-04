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
import java.util.Calendar;
import java.util.Enumeration;

import android.annotation.TargetApi;
import android.app.ActionBar;
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
import android.content.pm.ApplicationInfo;
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
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;
import android.widget.Toast;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonMainThread;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.stats.CollectorService;

public class Preferences extends PreferenceActivity {
	
	private final String TAG = "Preferences";
	
	private DTNService service = null;
	
	// progress dialog for the send process
	private ProgressDialog pd = null;
	
	private Switch actionBarSwitch = null;
	private CheckBoxPreference checkBoxPreference = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			Preferences.this.service = DTNService.Stub.asInterface(service);
			Log.i(TAG, "service connected");
			
			// on first startup ask for permissions to collect statistical data
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
			if (!prefs.contains("collect_stats")) {
				showStatisticLoggerDialog(Preferences.this);
			}
			
			// adjust daemon switch
			try {
                setDaemonSwitch(DaemonState.ONLINE.equals(Preferences.this.service.getState()));
            } catch (RemoteException e) {
                Log.e(TAG, "Can not query daemon state", e);
            }
		}

		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "service disconnected");
			service = null;
		}
	};
	
	public static void showStatisticLoggerDialog(final Activity activity) {
		DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
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
	
	@TargetApi(14)
	private void setDaemonSwitch(boolean val) {
		if (actionBarSwitch != null) {
			actionBarSwitch.setChecked(val);
		} else if (checkBoxPreference != null) {
			checkBoxPreference.setChecked(val);
		}
		
		setPrefsEnabled(!val);
	}
	
	@SuppressWarnings("deprecation")
	private void setPrefsEnabled(boolean val) {
		// enable / disable depending elements
		String[] prefcats = { "prefcat_general", "prefcat_interfaces", "prefcat_security", "prefcat_logging" };
		for (String pcat : prefcats) {
			PreferenceCategory pc = (PreferenceCategory) findPreference(pcat);
			pc.setEnabled(val);
		}
		
		String[] prefs = { "discovery_announce", "checkIdleTimeout" };
		for (String p : prefs) {
			Preference pobj = (Preference) findPreference(p);
			pobj.setEnabled(val);
		}
	}
	
	private BroadcastReceiver _state_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.STATE))
			{
				String state = intent.getStringExtra("state");
				DaemonState ds = DaemonState.valueOf(state);
				switch (ds)
				{
				case ONLINE:
					Preferences.this.setDaemonSwitch(true);
					break;
					
				case OFFLINE:
					Preferences.this.setDaemonSwitch(false);
					break;
					
				case ERROR:
					Preferences.this.setDaemonSwitch(false);
					break;
					
				default:
					break;
				}
			}
		}
	};
	
	private void setCloudUplink(boolean val) {
		Intent intent = new Intent(Preferences.this, DaemonService.class);
		intent.setAction(DaemonService.ACTION_CLOUD_UPLINK);
		intent.addCategory(Intent.CATEGORY_DEFAULT);
		intent.putExtra("enabled", val);
		startService(intent);
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
	    
	    if (0 != (getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE)) {
	    	menu.findItem(R.id.itemSendDataNow).setVisible(true);
	    } else {
	    	menu.findItem(R.id.itemSendDataNow).setVisible(false);
	    }
	    
	    return super.onCreateOptionsMenu(menu);
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
	    
	    case R.id.itemSendDataNow:
	    {
	    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
			Calendar now = Calendar.getInstance();
			prefs.edit().putLong("stats_timestamp", now.getTimeInMillis()).commit(); 

			// open activity
			Intent i = new Intent(this, CollectorService.class);
			i.setAction(CollectorService.DELIVER_DATA);
			startService(i);
	    	return true;
	    }
	    
	    case R.id.itemApps:
	    {
			Intent i = new Intent(Preferences.this, AppListActivity.class);
			startActivity(i);
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
			e.putString("endpoint_id", DaemonMainThread.getUniqueEndpointID(context).toString());
			
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
	
	@TargetApi(14)
	@SuppressWarnings("deprecation")
	@Override
	public void onCreate(Bundle savedInstanceState) {
		// initialize default values if configured set already
		initializeDefaultPreferences(this);
		
	    super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
		
		// connect daemon controls
        checkBoxPreference = (CheckBoxPreference) findPreference("enabledSwitch");
		if (checkBoxPreference != null) {
			checkBoxPreference.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference p) {
				if (((CheckBoxPreference) p).isChecked()) {
					Preferences.this.setPrefsEnabled(false);
					
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
		} else {
			// use custom actionbar switch
	        actionBarSwitch = new Switch(this);

	        //PreferenceActivity preferenceActivity = (PreferenceActivity) this;
	        //if (preferenceActivity.onIsHidingHeaders() || !preferenceActivity.onIsMultiPane()) {
	            final int padding = this.getResources().getDimensionPixelSize(
	                    R.dimen.action_bar_switch_padding);
	            actionBarSwitch.setPadding(0, 0, padding, 0);
	            this.getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
	                    ActionBar.DISPLAY_SHOW_CUSTOM);
	            this.getActionBar().setCustomView(actionBarSwitch, new ActionBar.LayoutParams(
	                    ActionBar.LayoutParams.WRAP_CONTENT,
	                    ActionBar.LayoutParams.WRAP_CONTENT,
	                    Gravity.CENTER_VERTICAL | Gravity.RIGHT));
	        //}
	        
	        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
	        
	        // read initial state of the switch
	        actionBarSwitch.setChecked( prefs.getBoolean("enabledSwitch", false) );
	        setPrefsEnabled( !prefs.getBoolean("enabledSwitch", false) );
	        
	        actionBarSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
				public void onCheckedChanged(CompoundButton arg0, boolean val) {
					Preferences.this.setPrefsEnabled(!val);
					
					if (val) {
						Preferences.this.setPrefsEnabled(false);
						
						// set "enabledSwitch" preference to true
						SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
						prefs.edit().putBoolean("enabledSwitch", true).commit();
						
						// startup the daemon process
						final Intent intent = new Intent(Preferences.this, DaemonService.class);
						intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_STARTUP);
						startService(intent);
					}
					else
					{
						// set "enabledSwitch" preference to false
						SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
						prefs.edit().putBoolean("enabledSwitch", false).commit();
						
						// shutdown the daemon
						final Intent intent = new Intent(Preferences.this, DaemonService.class);
						intent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_SHUTDOWN);
						startService(intent);
					}
				}
	        });
		}
		
		// add handle for cloud connect checkbox
		CheckBoxPreference cbCloudConnect = (CheckBoxPreference) findPreference("cloud_uplink");
		if (cbCloudConnect != null) {
			cbCloudConnect.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
			public boolean onPreferenceClick(Preference p) {
				setCloudUplink(((CheckBoxPreference) p).isChecked());
				return true;
			}
			});
		}
		
		// list all network interfaces
		try {
			PreferenceCategory pc = (PreferenceCategory) findPreference("prefcat_interfaces");
			
			for(Enumeration<NetworkInterface> list = NetworkInterface.getNetworkInterfaces(); list.hasMoreElements();)
		    {
	            NetworkInterface i = list.nextElement();
	            
	            // skip virtual interfaces
	            if (i.isVirtual()) continue;
	            
	            // do not work on non-multicast interfaces
	            if (!i.supportsMulticast()) continue;
	            
	            // skip loopback device
	            if (i.isLoopback()) continue;
	            
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
			version.setSummary(info.versionName);
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
