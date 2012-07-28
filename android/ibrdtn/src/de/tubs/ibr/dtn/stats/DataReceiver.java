package de.tubs.ibr.dtn.stats;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.TimeZone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;

public class DataReceiver extends BroadcastReceiver {
	
	private final long LIMIT_FILESIZE = 500000;
//	private final static String TAG = "DataReceiver";
	public static Object datalock = new Object(); 
	
	@Override
	public void onReceive(Context context, Intent intent) {
		if (intent.getAction().equals(de.tubs.ibr.dtn.Intent.EVENT)) {
			
			// check if enabled
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
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
			storeData(context, xmlData);
		}
	}
	
	private void storeData(Context context, String data) {
		synchronized(datalock) {
			try {
				FileOutputStream output = context.openFileOutput("events.dat", Context.MODE_PRIVATE | Context.MODE_APPEND);
				output.write(data.getBytes());
				output.close();
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			// check if the last statistic bundle was send at least two hours ago
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			Calendar calendar = Calendar.getInstance();
			calendar.roll(Calendar.HOUR, false);
			
			if (calendar.getTimeInMillis() < prefs.getLong("stats_timestamp", 0)) return;
						
			File ef = new File(context.getFilesDir().getPath() + File.separatorChar + "events.dat");
			//if (ef.exists()) Log.d(TAG, "File size: " + Long.toString( ef.length() ));
			
			// compress and send the log file if the size is too large
			if (ef.length() > LIMIT_FILESIZE) {
				Calendar now = Calendar.getInstance();
				prefs.edit().putLong("stats_timestamp", now.getTimeInMillis()).commit(); 

				// open activity
				Intent i = new Intent(context, CollectorService.class);
				i.setAction(CollectorService.DELIVER_DATA);
				i.setData(Uri.fromFile(ef));
				context.startService(i);
			}
		}
	}
}
