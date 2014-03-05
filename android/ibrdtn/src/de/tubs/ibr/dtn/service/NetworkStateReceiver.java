package de.tubs.ibr.dtn.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.preference.PreferenceManager;
import de.tubs.ibr.dtn.daemon.Preferences;

public class NetworkStateReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(final Context context, final Intent intent) {
		final ConnectivityManager connMgr = (ConnectivityManager) 
				context.getSystemService(Context.CONNECTIVITY_SERVICE);

		final android.net.NetworkInfo wifi = 
				connMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
		
		// forward to the daemon if it is enabled
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		if (prefs.getBoolean(Preferences.KEY_ENABLED, true) && wifi.isConnected()) {
			// tickle the daemon service
			final Intent wakeUpIntent = new Intent(context, DaemonService.class);
			wakeUpIntent.setAction(de.tubs.ibr.dtn.service.DaemonService.ACTION_NETWORK_CHANGED);
			context.startService(wakeUpIntent);
		}
	}
}
