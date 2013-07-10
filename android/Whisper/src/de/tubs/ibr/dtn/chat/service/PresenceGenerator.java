/*
 * AlarmReceiver.java
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
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class PresenceGenerator {
    
    private static final String TAG = "PresenceGenerator";

    // activate alarm every 15 minutes
    static public void activate(Context context)
    {
        // create a new wakeup intent
        Intent intent = new Intent(context, ChatService.class);
        intent.setAction(ChatService.ACTION_PRESENCE_ALARM);
        
        // check if the presence alarm is already active
        PendingIntent pi = PendingIntent.getService(context, 0, intent, PendingIntent.FLAG_NO_CREATE);
        
        if (pi == null) {
            // create pending intent
            PendingIntent sender = PendingIntent.getService(context, 0, intent, PendingIntent.FLAG_CANCEL_CURRENT);
    
            // Get the AlarmManager service
            AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            am.setInexactRepeating(AlarmManager.RTC_WAKEUP, new Date().getTime(), AlarmManager.INTERVAL_FIFTEEN_MINUTES, sender);
            
            Log.i(TAG, "Alarm set for every 15 minutes.");
        } else {
            Log.i(TAG, "Alarm already active.");
        }
    }
    
    // deactivate alarm every 15 minutes
    static public void deactivate(Context context)
    {
        // create a new wakeup intent
        Intent intent = new Intent(context, ChatService.class);
        intent.setAction(ChatService.ACTION_PRESENCE_ALARM);
        
        // check if the presence alarm is already active
        PendingIntent pi = PendingIntent.getService(context, 0, intent, PendingIntent.FLAG_NO_CREATE);
        
        if (pi != null) {
            // Get the AlarmManager service
            AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
            am.cancel(pi);
            pi.cancel();
            
            Log.i(TAG, "Alarm canceled.");
        }
    }
}
