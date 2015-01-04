/*
 * EventReceiver.java
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
package de.tubs.ibr.dtn.chat.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

public class EventReceiver extends BroadcastReceiver {
	
	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		
		if (action.equals(de.tubs.ibr.dtn.Intent.RECEIVE))
		{
			// start receiving service
			Intent i = new Intent(context, ChatService.class);
			i.setAction(de.tubs.ibr.dtn.Intent.RECEIVE);
			context.startService(i);
		}
		else if (action.equals(de.tubs.ibr.dtn.Intent.STATUS_REPORT))
		{
			// forward intent to the chat service
			Intent i = new Intent(context, ChatService.class);
			i.setAction(ChatService.REPORT_DELIVERED_INTENT);
			i.putExtra("source", intent.getParcelableExtra("source"));
			i.putExtra("bundleid", intent.getParcelableExtra("bundleid"));
			context.startService(i);
		}
		else if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) ||
				 Intent.ACTION_MY_PACKAGE_REPLACED.equals(intent.getAction()))
		{
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			String desired = prefs.getString("presencetag", "unavailable");
			
			if (desired.equals("unavailable")) {
				// deactivate presence generator
				PresenceGenerator.deactivate(context);
			} else {
				// activate presence generator
				PresenceGenerator.activate(context);
				
				// send initial presence
				Intent presenceIntent = new Intent(context, ChatService.class);
				presenceIntent.setAction(ChatService.ACTION_PRESENCE_ALARM);
				context.startService(presenceIntent);
			}
		}
	}
}
