package de.tubs.ibr.dtn.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

public class IntentReceiver extends BroadcastReceiver {
	
	private final static String TAG = "IntentReceiver";

	@Override
	public void onReceive(Context context, Intent intent) {
		// load preferences
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
		Log.i(TAG, "IntentReceiver called by " + intent.getAction());
		
		if (intent.getAction().equals("android.intent.action.BOOT_COMPLETED"))
		{
			if (preferences.getBoolean("runonboot", false))
			{
				// start the dtnd service
				Intent is = new Intent(context, DaemonService.class);
				is.setAction(DaemonService.ACTION_STARTUP);
				context.startService(is);
			}
		}
	}
}