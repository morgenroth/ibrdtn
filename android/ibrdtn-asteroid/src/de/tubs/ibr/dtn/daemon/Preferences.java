package de.tubs.ibr.dtn.daemon;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import de.tubs.ibr.dtn.DTNService;
import de.tubs.ibr.dtn.DaemonState;
import de.tubs.ibr.dtn.R;
import de.tubs.ibr.dtn.service.DaemonService;

public class Preferences extends PreferenceActivity {
	
	private final String TAG = "Preferences";
	
	private DTNService service = null;
	
	private ServiceConnection mConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			Preferences.this.service = DTNService.Stub.asInterface(service);
			Log.i(TAG, "Preferences: service connected");
			
			CheckBoxPreference checkActivate = (CheckBoxPreference) findPreference("checkActivate");
			try {
				checkActivate.setChecked(Preferences.this.service.getState() == DaemonState.ONLINE);
			} catch (RemoteException e) {
				checkActivate.setChecked(false);
			}
			
			(new CheckDaemonState()).execute();
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.i(TAG, "Preferences: service disconnected");
			service = null;
		}
	};
	
	private BroadcastReceiver _state_receiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.STATE))
			{
				CheckBoxPreference checkActivate = (CheckBoxPreference) findPreference("checkActivate");
				
				String state = intent.getStringExtra("state");
				DaemonState ds = DaemonState.valueOf(state);
				switch (ds)
				{
				case ONLINE:
					checkActivate.setChecked(true);
					break;
					
				case OFFLINE:
					checkActivate.setChecked(false);
					break;
					
				case ERROR:
					checkActivate.setChecked(false);
					break;
				}
				
				checkActivate.setEnabled(true);
			}
		}
	};
	
	private class CheckDaemonState extends AsyncTask<String, Integer, Boolean> {
		protected Boolean doInBackground(String... data)
		{
			try {
				return Preferences.this.service.isRunning();
			} catch (Exception e) {
				return false;
			}
		}

		protected void onProgressUpdate(Integer... progress) {
		}

		protected void onPostExecute(Boolean result)
		{
			CheckBoxPreference checkActivate = (CheckBoxPreference) findPreference("checkActivate");
			checkActivate.setEnabled(true);
			checkActivate.setChecked(result);
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.main_menu, menu);
	    return true;
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
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
		
		// disable daemon controls
		Preference checkActivate = (Preference) findPreference("checkActivate");
		checkActivate.setEnabled(false);
		checkActivate.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
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
