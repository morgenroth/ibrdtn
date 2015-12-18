/*
 * IntentReceiver.java
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;
import de.tubs.ibr.dtn.daemon.Preferences;

public class OnRestartReceiver extends BroadcastReceiver {
	
	private final static String TAG = "OnRestartReceiver";

	@Override
	public void onReceive(Context context, Intent intent) {
		// load preferences
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
		
		if (	Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) ||
				Intent.ACTION_MY_PACKAGE_REPLACED.equals(intent.getAction())	)
		{
			if (preferences.getBoolean(Preferences.KEY_ENABLED, true))
			{
				Log.d(TAG, "Restarted due to " + intent.getAction());
				
				// start the dtnd service
				Intent is = new Intent(context, DaemonService.class);
				is.setAction(DaemonService.ACTION_STARTUP);
				context.startService(is);
			}
		}
	}
}