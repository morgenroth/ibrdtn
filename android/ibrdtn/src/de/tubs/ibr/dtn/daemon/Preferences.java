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

import java.util.Calendar;

import android.annotation.TargetApi;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
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
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonProcess;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.stats.CollectorService;

public class Preferences extends PreferenceActivity {
	
	private final String TAG = "Preferences";
	
	// These preferences show their value as summary
	private final static String[] mSummaryPrefs = { "endpoint_id", "routing", "security_mode", "log_options", "log_debug_verbosity" };
	
	private Boolean mBound = false;
	private DTNService service = null;
	
	private Switch actionBarSwitch = null;
	private CheckBoxPreference checkBoxPreference = null;
	private InterfacePreferenceCategory mInterfacePreference = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName name, IBinder service) {
			Preferences.this.service = DTNService.Stub.asInterface(service);
			if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service connected");
				
			// get the daemon version
			try {
			    String version[] = Preferences.this.service.getVersion();
			    setVersion("dtnd: " + version[0] + ", build: " + version[1]);
            } catch (RemoteException e) {
                Log.e(TAG, "Can not query the daemon version", e);
            }
		}

		public void onServiceDisconnected(ComponentName name) {
		    if (Log.isLoggable(TAG, Log.DEBUG)) Log.d(TAG, "service disconnected");
			service = null;
		}
	};
	
	public static void showStatisticLoggerDialog(final Activity activity) {
		DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
		    public void onClick(DialogInterface dialog, int which) {
		    	SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(activity);
		        PreferenceActivity prefactivity = (PreferenceActivity)activity;
		        
		        @SuppressWarnings("deprecation")
				CheckBoxPreference cb = (CheckBoxPreference)prefactivity.findPreference("collect_stats");
		    	
		        switch (which){
		        case DialogInterface.BUTTON_POSITIVE:
		        	prefs.edit().putBoolean("collect_stats", true).putBoolean("collect_stats_initialized", true).commit();
		        	cb.setChecked(true);
		            break;

		        case DialogInterface.BUTTON_NEGATIVE:
		        	prefs.edit().putBoolean("collect_stats", false).putBoolean("collect_stats_initialized", true).commit();
		        	cb.setChecked(false);
		            break;
		        }
		    }
		};

		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setTitle(R.string.alert_statistic_logger_title);
		builder.setMessage(activity.getResources().getString(R.string.alert_statistic_logger_dialog));
		builder.setPositiveButton(activity.getResources().getString(android.R.string.yes), dialogClickListener);
		builder.setNegativeButton(activity.getResources().getString(android.R.string.no), dialogClickListener);
		builder.show();
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
			Intent i = new Intent(Preferences.this, DaemonService.class);
			i.setAction(DaemonService.ACTION_CLEAR_STORAGE);
			startService(i);
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
	    	Intent i = new Intent(Preferences.this, NeighborActivity.class);
	    	startActivity(i);
	    	return true;
	    }
	    
	    default:
	        return super.onOptionsItemSelected(item);
	    }
	}
	
	public static void initializeDefaultPreferences(Context context) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		
		if (prefs.getBoolean("initialized", false)) return;

		Editor e = prefs.edit();
		e.putString("endpoint_id", DaemonProcess.getUniqueEndpointID(context).toString());
		
		// set preferences to initialized
		e.putBoolean("initialized", true);
		
		e.commit();
	}

	@TargetApi(14)
	@SuppressWarnings("deprecation")
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// set default preference values
		PreferenceManager.setDefaultValues(this, R.xml.preferences, false);
		
		// initialize default values if configured set already
		initializeDefaultPreferences(this);

		addPreferencesFromResource(R.xml.preferences);
		
		mInterfacePreference = (InterfacePreferenceCategory)findPreference("prefcat_interfaces");
		
		// connect daemon controls
        checkBoxPreference = (CheckBoxPreference) findPreference("enabledSwitch");
        if (checkBoxPreference == null) {
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
	        
	        actionBarSwitch.setOnCheckedChangeListener(new OnCheckedChangeListener() {
				public void onCheckedChanged(CompoundButton arg0, boolean val) {

					if (val) {
						// set "enabledSwitch" preference to true
						SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
						prefs.edit().putBoolean("enabledSwitch", true).commit();
					}
					else
					{
						// set "enabledSwitch" preference to false
						SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
						prefs.edit().putBoolean("enabledSwitch", false).commit();
					}
				}
	        });
		}

		// set initial version
		setVersion(null);
		
        // Bind the summaries of EditText/List/Dialog/Ringtone preferences to
        // their values. When their values change, their summaries are updated
        // to reflect the new value, per the Android Design guidelines.
		for (String prefKey : mSummaryPrefs) {
		    bindPreferenceSummaryToValue(findPreference(prefKey));
		}
	}
	
	@Override
	public void onDestroy() {
	    if (mBound) {
	        // Detach our existing connection.
	        unbindService(mConnection);
	        mBound = false;
	    }

	    super.onDestroy();
	}
	
	@SuppressWarnings("deprecation")
	private void setVersion(String versionValue) {
        // version information
        Preference version = findPreference("system_version");
        try {
            PackageInfo info = getPackageManager().getPackageInfo(getPackageName(), 0);
            if (versionValue == null) {
                version.setSummary("app: " + info.versionName);
            } else {
                version.setSummary("app: " + info.versionName + ", " + versionValue);
            }
        } catch (NameNotFoundException e) { };
	}
	
    @Override
	protected void onPause() {
		super.onPause();
		
		unregisterReceiver(mNetworkConditionListener);
	}

	@Override
	protected void onResume() {
	    if (!mBound) {
    		// Establish a connection with the service.  We use an explicit
    		// class name because we want a specific service implementation that
    		// we know will be running in our own process (and thus won't be
    		// supporting component replacement by other applications).
    		bindService(new Intent(Preferences.this, 
    				DaemonService.class), mConnection, Context.BIND_AUTO_CREATE);
    		mBound = true;
	    }
  		
		super.onResume();
		
		// on first startup ask for permissions to collect statistical data
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(Preferences.this);
		if (!prefs.getBoolean("collect_stats_initialized", false)) {
			showStatisticLoggerDialog(Preferences.this);
		}
		
        IntentFilter filter =new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION);
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        registerReceiver(mNetworkConditionListener, filter);
        
        mInterfacePreference.updateInterfaceList();
	}
	
    private BroadcastReceiver mNetworkConditionListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mInterfacePreference.updateInterfaceList();
                }
            });
        }
    };
    
    /**
     * A preference value change listener that updates the preference's summary
     * to reflect its new value.
     */
    private static Preference.OnPreferenceChangeListener sBindPreferenceSummaryToValueListener = new Preference.OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object value) {
            String stringValue = value.toString();
            
            for (String prefKey : mSummaryPrefs) {
                if (prefKey.equals(preference.getKey())) {
                    if (preference instanceof ListPreference) {
                        // For list preferences, look up the correct display value in
                        // the preference's 'entries' list.
                        ListPreference listPreference = (ListPreference) preference;
                        int index = listPreference.findIndexOfValue(stringValue);

                        // Set the summary to reflect the new value.
                        preference.setSummary(
                                index >= 0
                                        ? listPreference.getEntries()[index]
                                        : null);

                    } else {
                        // For all other preferences, set the summary to the value's
                        // simple string representation.
                        preference.setSummary(stringValue);
                    }
                    return true;
                }
            }
            
            return true;
        }
    };

    /**
     * Binds a preference's summary to its value. More specifically, when the
     * preference's value is changed, its summary (line of text below the
     * preference title) is updated to reflect the value. The summary is also
     * immediately updated upon calling this method. The exact display format is
     * dependent on the type of preference.
     * 
     * @see #sBindPreferenceSummaryToValueListener
     */
    private static void bindPreferenceSummaryToValue(Preference preference) {
        // Set the listener to watch for value changes.
        preference.setOnPreferenceChangeListener(sBindPreferenceSummaryToValueListener);

        // Trigger the listener immediately with the preference's
        // current value.
        sBindPreferenceSummaryToValueListener.onPreferenceChange(preference,
                PreferenceManager
                        .getDefaultSharedPreferences(preference.getContext())
                        .getString(preference.getKey(), ""));
    }
}
