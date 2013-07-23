package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.util.Log;
import de.tubs.ibr.dtn.p2p.scheduler.SchedulerService;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.swig.EID;
import de.tubs.ibr.dtn.swig.NativeP2pManager;

@TargetApi(16)
public class P2pManager extends NativeP2pManager {

    private final static String TAG = "P2PManager";

    private DaemonService mService = null;

    /**
     * How to control the P2P API of the daemon
     * 
     * The following control calls are only possible if the client is connected
     * to the daemon. That means, do such a call only if "initialize" was
     * successful and "destroy" has not been called yet.
     * 
     * Call a "fireDiscovered" event for each discovered peer earlier than every
     * 120 seconds. fireDiscovered(new
     * ibrdtn.api.object.SingletonEndpoint("dtn://test-node"), data);
     * 
     * If a new interface has been created due to a P2P connection, then call
     * the "fireInterfaceUp" event. fireInterfaceUp("wlan0-p2p0");
     * 
     * If a P2P interface goes down then call "fireInterfaceDown".
     * fireInterfaceDown("wlan0-p2p0");
     * 
     */

    public P2pManager(DaemonService service) {
        super("P2P:WIFI");
        this.mService = service;
    }

    public void initialize() {
        // daemon is up
        
        // listen to wifi p2p events
        IntentFilter filter = new IntentFilter();
        filter.addAction("android.net.wifi.p2p.CONNECTION_STATE_CHANGE");
        filter.addAction("android.net.wifi.p2p.PEERS_CHANGED");
        filter.addAction("android.net.wifi.p2p.STATE_CHANGED");
        filter.addAction("android.net.wifi.p2p.DISCOVERY_STATE_CHANGE");
        mService.registerReceiver(mWifiEventReceiver, filter);
    }
    
    public void resume() {
        // start the scheduler
        Intent i = new Intent(mService, SchedulerService.class);
        i.setAction(SchedulerService.ACTION_CHECK_STATE);
        mService.startService(i);
    }
    
    public void pause() {
        // stop the scheduler
        Intent i = new Intent(mService, SchedulerService.class);
        i.setAction(SchedulerService.ACTION_STOP_SCHEDULER);
        mService.startService(i);
    }

    public void destroy() {
        // daemon goes down
        
        // stop listening to wifi p2p events
        mService.unregisterReceiver(mWifiEventReceiver);
    }

    public void fireInterfaceUp(String iface) {
        Log.d(TAG, "interface up:" + iface);
        super.fireInterfaceUp(iface);
    }

    public void fireInterfaceDown(String iface) {
        Log.d(TAG, "interface down:" + iface);
        super.fireInterfaceDown(iface);
    }

    public void fireDiscovered(String eid, String identifier) {
        Log.d(TAG, "found peer: " + eid + ", " + identifier);
        super.fireDiscovered(new EID(eid), identifier, 90, 10);
    }

    @Override
    public void connect(String data) {
        // connect to the peer identified by "data"
        Intent i = new Intent(mService, WifiP2pService.class);
        i.setAction(WifiP2pService.CONNECT_TO_PEER_ACTION);
        i.putExtra(WifiP2pService.MAC_EXTRA, data);
        mService.startService(i);
        Log.i(TAG, "connect request: " + data);
    }

    @Override
    public void disconnect(String data) {
        // disconnect from the peer identified by "data"
        Intent i = new Intent(mService, WifiP2pService.class);
        i.setAction(WifiP2pService.DISCONNECT_FROM_PEER_ACTION);
        i.putExtra(WifiP2pService.MAC_EXTRA, data);
        mService.startService(i);
        Log.i(TAG, "disconnect request: " + data);
    }
    
    public BroadcastReceiver mWifiEventReceiver = new BroadcastReceiver() {

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
                Intent i = new Intent(context, WifiP2pService.class);
                i.setAction(WifiP2pService.CONNECTED_TO_PEER);
                context.startService(i);
            }
        }

        private void peersChanged(Context context, Intent intent) {
            Intent i = new Intent(context, WifiP2pService.class);
            i.setAction(WifiP2pService.PEERS_CHANGED_ACTION);
            context.startService(i);
        }

        private void discoveryChanged(Context context, Intent intent) {
            int discoveryState = intent.getIntExtra(
                    WifiP2pManager.EXTRA_DISCOVERY_STATE,
                    WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED);
            if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED) {
                Intent i = new Intent(context, SchedulerService.class);
                i.setAction(SchedulerService.ACTION_CHECK_STATE);
                context.startService(i);
            } else {
            }
        }
    };
}
