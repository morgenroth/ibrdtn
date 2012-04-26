package de.tubs.ibr.dtn.chat.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class AlarmReceiver extends BroadcastReceiver {

	public final static Integer REQUEST_CODE = 4242;

	public final static String ACTION = "de.tubs.ibr.dtn.chat.PRESENCE_ALARM";
	
	@Override
	public void onReceive(Context context, Intent intent) {
		// wake-up the chat service and queue a send presence task
		Intent i = new Intent(context, ChatService.class);
		i.setAction(ACTION);
		context.startService(i);
	}
}
