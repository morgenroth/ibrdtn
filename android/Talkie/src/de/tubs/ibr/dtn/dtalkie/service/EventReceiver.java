package de.tubs.ibr.dtn.dtalkie.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
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
				// open activity
				Intent i = new Intent(context, DTalkieService.class);
				i.setAction(de.tubs.ibr.dtn.Intent.RECEIVE);
				context.startService(i);
			}
			else if (state.equals("OFFLINE"))
			{
			}
		}
		else if (action.equals(de.tubs.ibr.dtn.Intent.RECEIVE))
		{
			// open activity
			Intent i = new Intent(context, DTalkieService.class);
			i.setAction(de.tubs.ibr.dtn.Intent.RECEIVE);
			context.startService(i);
		}
	}
}
