package de.tubs.ibr.dtn.p2p.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;

public class WiFiReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
			// TODO
		}
	}

}
