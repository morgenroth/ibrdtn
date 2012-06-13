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

import java.util.Date;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.preference.PreferenceManager;
import android.util.Log;

public class EventReceiver extends BroadcastReceiver {
	
	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		
		Log.d("EventReceiver", "Intent received: " + action);
		
		if (action.equals(de.tubs.ibr.dtn.Intent.STATE))
		{
			String state = intent.getStringExtra("state");
			Log.d("EventReceiver", "State: " + state);
			if (state.equals("ONLINE"))
			{
				// respect user settings
				if (PreferenceManager.getDefaultSharedPreferences(context).getBoolean("checkBroadcastPresence", false))
				{
					// register scheduled presence update
					activatePresenceGenerator(context);
					
					// wake-up the chat service and queue a send presence task
					Intent i = new Intent(context, ChatService.class);
					i.setAction(AlarmReceiver.ACTION);
					context.startService(i);
				}
				
				// open activity
				Intent i = new Intent(context, ChatService.class);
				i.setAction(de.tubs.ibr.dtn.Intent.RECEIVE);
				context.startService(i);
			}
			else if (state.equals("OFFLINE"))
			{
				// unregister scheduled presence update
				deactivatePresenceGenerator(context);
			}
		}
		else if (action.equals(de.tubs.ibr.dtn.Intent.REGISTRATION))
		{
			// registration successful
		}
		else if (action.equals(de.tubs.ibr.dtn.Intent.RECEIVE))
		{
			// open activity
			Intent i = new Intent(context, ChatService.class);
			i.setAction(de.tubs.ibr.dtn.Intent.RECEIVE);
			context.startService(i);
		}
	}
	
	// activate alarm every 15 minutes
	static public void activatePresenceGenerator(Context context)
	{	
		// create a new wakeup intent
		Intent intent = new Intent(context, AlarmReceiver.class);
		
		// create pending intent
		PendingIntent sender = PendingIntent.getBroadcast(context, AlarmReceiver.REQUEST_CODE, intent, PendingIntent.FLAG_UPDATE_CURRENT);

		// Get the AlarmManager service
		AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
		am.setInexactRepeating(AlarmManager.RTC_WAKEUP, new Date().getTime(), AlarmManager.INTERVAL_FIFTEEN_MINUTES, sender);
		
		Log.i("EventReceiver", "Presence alarm set for every 15 minutes.");
	}
	
	// deactivate alarm every 15 minutes
	static public void deactivatePresenceGenerator(Context context)
	{	
		// create a new wakeup intent
		Intent intent = new Intent(context, AlarmReceiver.class);
		
		// create pending intent
		PendingIntent sender = PendingIntent.getBroadcast(context, AlarmReceiver.REQUEST_CODE, intent, PendingIntent.FLAG_UPDATE_CURRENT);

		// Get the AlarmManager service
		AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
		am.cancel(sender);
		
		Log.i("EventReceiver", "Presence alarm canceled.");
	}
}
