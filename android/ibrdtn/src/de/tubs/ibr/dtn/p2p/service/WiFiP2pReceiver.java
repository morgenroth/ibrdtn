package de.tubs.ibr.dtn.p2p.service;

import android.annotation.*;
import android.content.*;
import android.net.*;
import android.net.wifi.p2p.*;
import android.util.*;
import de.tubs.ibr.dtn.p2p.scheduler.*;

@TargetApi(16)
public class WiFiP2pReceiver extends BroadcastReceiver {

    public static final String TAG = "WiFiP2pReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        if (WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
            discoveryChanged(context, intent);
        } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
            peersChanged(context, intent);
        } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION
                .equals(action)) {
            connectionChanged(context, intent);
        } else if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
            // TODO
        }
    }

    private void connectionChanged(Context context, Intent intent) {
        NetworkInfo netInfo = intent
                .getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
        Log.d(TAG, "DetailedState: " + netInfo.getDetailedState());
        if (netInfo.isConnected()) {
            Intent i = new Intent(context, WiFiP2P4IbrDtnService.class);
            i.setAction(WiFiP2P4IbrDtnService.CONNECTED_TO_PEER);
            context.startService(i);
        }
    }

    // private void connectToPeer(Context context, Intent intent) {
    // String mac = intent.getStringExtra(WiFiP2P4IbrDtnService.MAC_EXTRA);
    // Intent i = new Intent(context, WiFiP2P4IbrDtnService.class);
    // i.setAction(WiFiP2P4IbrDtnService.CONNECT_TO_PEER_ACTION);
    // i.putExtra(WiFiP2P4IbrDtnService.MAC_EXTRA, mac);
    // context.startService(i);
    //
    // }
    //
    // private void peerFound(Context context, Intent intent) {
    // String mac = intent.getStringExtra(WiFiP2P4IbrDtnService.MAC_EXTRA);
    // Intent i = new Intent(WiFiP2P4IbrDtnService.CONNECT_TO_PEER_ACTION);
    // i.putExtra(WiFiP2P4IbrDtnService.MAC_EXTRA, mac);
    // context.sendBroadcast(i);
    // }

    private void peersChanged(Context context, Intent intent) {
        Intent i = new Intent(context, WiFiP2P4IbrDtnService.class);
        i.setAction(WiFiP2P4IbrDtnService.PEERS_CHANGED_ACTION);
        context.startService(i);
    }

    private void discoveryChanged(Context context, Intent intent) {
        int discoveryState = intent.getIntExtra(
                WifiP2pManager.EXTRA_DISCOVERY_STATE,
                WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED);
        if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED) {
            Intent i = new Intent(context, SchedulerService.class);
            i.setAction(AlarmReceiver.ACTION);
            context.startService(i);
        } else {
        }

    }

}
