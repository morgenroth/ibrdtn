package de.tubs.ibr.dtn.stats;

import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.TimeZone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import de.tubs.ibr.dtn.service.DaemonService;

public class DataReceiver extends BroadcastReceiver {
	
	@Override
	public void onReceive(Context context, Intent intent) {
		if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.EVENT)) {
			
			// check if enabled
			SharedPreferences prefs = DaemonService.getSharedPreferences(context);
			if (!prefs.getBoolean("collect_stats", false)) return;
			
			String eventName = intent.getStringExtra("name");
			String eventAction = intent.getStringExtra("action");
			
			String xmlData = "<event name=\"" + eventName + "\"";			
					
			if (eventAction != null) xmlData += " action=\"" + eventAction + "\"";
			
			Calendar gmt = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
			String timestamp = Long.toString( gmt.getTimeInMillis() );
			xmlData += " timestamp=\"" + timestamp + "\">";
			
			for (String key : intent.getExtras().keySet()) {
				if (key.startsWith("attr:")) {
					xmlData += "<" + key.substring(5) + ">" + intent.getStringExtra(key) + "</" + key.substring(5) + ">";
				}
			}			
			
			xmlData += "</event>";
			
			Intent i = new Intent(context, CollectorService.class);
			i.setAction(CollectorService.REC_DATA);
			i.putExtra("xmldata", xmlData);
			context.startService(i);
		}
	}
}
