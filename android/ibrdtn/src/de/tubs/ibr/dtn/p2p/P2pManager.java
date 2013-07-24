package de.tubs.ibr.dtn.p2p;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.NetworkInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.DnsSdServiceResponseListener;
import android.net.wifi.p2p.WifiP2pManager.DnsSdTxtRecordListener;
import android.net.wifi.p2p.WifiP2pManager.GroupInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.net.wifi.p2p.nsd.WifiP2pDnsSdServiceInfo;
import android.net.wifi.p2p.nsd.WifiP2pDnsSdServiceRequest;
import android.preference.PreferenceManager;
import android.util.Log;
import de.tubs.ibr.dtn.p2p.db.Database;
import de.tubs.ibr.dtn.p2p.db.Peer;
import de.tubs.ibr.dtn.p2p.scheduler.SchedulerService;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.swig.EID;
import de.tubs.ibr.dtn.swig.NativeP2pManager;

@TargetApi(16)
public class P2pManager extends NativeP2pManager {

    private final static String TAG = "P2PManager";
    
    public static final String START_DISCOVERY_ACTION = "de.tubs.ibr.dtn.p2p.action.START_DISCOVERY";
    public static final String STOP_DISCOVERY_ACTION = "de.tubs.ibr.dtn.p2p.action.STOP_DISCOVERY";
    
    public static final String PEER_FOUND_ACTION = "de.tubs.ibr.dtn.p2p.action.PEER_FOUND";
    public static final String EID_EXTRA = "de.tubs.ibr.dtn.p2p.extra.EID";
    public static final String MAC_EXTRA = "de.tubs.ibr.dtn.p2p.extra.MAC";
    
    public static final String CONNECTION_CHANGED_ACTION = "de.tubs.ibr.dtn.p2p.action.CONNECTION_CHANGED";
    public static final String STATE_EXTRA = "de.tubs.ibr.dtn.p2p.extra.STATE";
    public static final String INTERFACE_EXTRA = "de.tubs.ibr.dtn.p2p.extra.INTERFACE";

    private DaemonService mService = null;
    
    private WifiP2pManager mWifiP2pManager = null;
    private WifiP2pManager.Channel mWifiP2pChannel = null;
    private WifiP2pDnsSdServiceRequest mServiceRequest = null;
    private WifiP2pDnsSdServiceInfo mServiceInfo = null;

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
    
    @SuppressWarnings("unused")
    private WifiP2pManager.ChannelListener mChannelListener = new WifiP2pManager.ChannelListener() {
        @Override
        public void onChannelDisconnected() {
            onPause();
            onDestroy();
            
            // disable P2P switch
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mService);
            prefs.edit().putBoolean(SettingsUtil.KEY_P2P_ENABLED, false).commit();
        }
    };

    public void onCreate() {
        // daemon is up
        
        // get the WifiP2pManager
        mWifiP2pManager = (WifiP2pManager) mService.getSystemService(Context.WIFI_P2P_SERVICE);
        
        // create a new Wi-Fi P2P channel
        //mWifiP2pChannel = mWifiP2pManager.initialize(mService, mService.getMainLooper(), mChannelListener);
        
        // channel listener disabled - this causes side effects: when terminating the whole app the wifi p2p
        // switch if set to off until a user set it to on again
        mWifiP2pChannel = mWifiP2pManager.initialize(mService, mService.getMainLooper(), null);
        
        // set service discovery listener
        mWifiP2pManager.setDnsSdResponseListeners(mWifiP2pChannel, mServiceResponseListener, mRecordListener);
        
        // create local service info
        Map<String, String> record = new HashMap<String, String>();
        record.put("eid", SettingsUtil.getEid(mService));
        mServiceInfo = WifiP2pDnsSdServiceInfo.newInstance("_dtn", "_presence._tcp", record);
        
        // listen to wifi p2p events
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
        filter.addAction(START_DISCOVERY_ACTION);
        filter.addAction(STOP_DISCOVERY_ACTION);
        mService.registerReceiver(mWifiEventReceiver, filter);
    }
    
    public void onResume() {
        if (mWifiP2pManager != null) {
            // start the scheduler
            Intent i = new Intent(mService, SchedulerService.class);
            i.setAction(SchedulerService.ACTION_ACTIVATE_SCHEDULER);
            mService.startService(i);
            
            // add local service description
            mWifiP2pManager.addLocalService(mWifiP2pChannel, mServiceInfo, null);
            
            // add service discovery request
            mServiceRequest = WifiP2pDnsSdServiceRequest.newInstance();
            mWifiP2pManager.addServiceRequest(mWifiP2pChannel, mServiceRequest, null);
        }
    }
    
    public void onPause() {
        if (mWifiP2pManager != null) {
            // stop the scheduler
            Intent i = new Intent(mService, SchedulerService.class);
            i.setAction(SchedulerService.ACTION_DEACTIVATE_SCHEDULER);
            mService.startService(i);
            
            // clear all service discovery requests
            mWifiP2pManager.clearServiceRequests(mWifiP2pChannel, null);
            
            // remove local service description
            mWifiP2pManager.clearLocalServices(mWifiP2pChannel, null);
            
            // stop all peer discoveries
            mWifiP2pManager.stopPeerDiscovery(mWifiP2pChannel, null);
        }
    }

    public void onDestroy() {
        if (mWifiP2pManager != null) {
            // stop listening to wifi p2p events
            mService.unregisterReceiver(mWifiEventReceiver);
        }
        
        // daemon goes down
        mServiceInfo = null;
        mWifiP2pChannel = null;
        mWifiP2pManager = null;
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
        Log.i(TAG, "connect request: " + data);

        // connect to the peer identified by "data"
        WifiP2pConfig config = new WifiP2pConfig();
        config.deviceAddress = data;
        mWifiP2pManager.connect(mWifiP2pChannel, config, null);
    }

    @Override
    public void disconnect(String data) {
        // disconnect from the peer identified by "data"
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
            } else if (START_DISCOVERY_ACTION.equals(action)) {
                mWifiP2pManager.discoverPeers(mWifiP2pChannel, null);
            } else if (STOP_DISCOVERY_ACTION.equals(action)) {
                mWifiP2pManager.stopPeerDiscovery(mWifiP2pChannel, null);
            }
        }
        
        private void connectionChanged(Context context, Intent intent) {
            NetworkInfo netInfo = intent
                    .getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
            Log.d(TAG, "DetailedState: " + netInfo.getDetailedState());
            if (netInfo.isConnected()) {
                P2pManager.this.connectedToPeer();
            }
        }

        private void peersChanged(Context context, Intent intent) {
            P2pManager.this.peersChanged();
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
    
    private void connectedToPeer() {
        mWifiP2pManager.requestGroupInfo(mWifiP2pChannel,
                new GroupInfoListener() {
                    @Override
                    public void onGroupInfoAvailable(WifiP2pGroup group) {
                        String iface = group.getInterface();

                        Intent i = new Intent(mService, DaemonService.class);
                        i.setAction(CONNECTION_CHANGED_ACTION);
                        i.putExtra(STATE_EXTRA, 1);
                        i.putExtra(INTERFACE_EXTRA, iface);
                        mService.startService(i);
                    }
                });
    }

    private void peersChanged() {
        mWifiP2pManager.requestPeers(mWifiP2pChannel, new PeerListListener() {
            @Override
            public void onPeersAvailable(WifiP2pDeviceList peers) {
                Log.d(TAG, "Peers available:" + peers.getDeviceList().size());
                P2pManager.this.checkPeers(peers);
            }
        });
    }

    private void checkPeers(WifiP2pDeviceList peers) {
        boolean allEIDs = true;
        for (WifiP2pDevice device : peers.getDeviceList()) {
            Database db = Database.getInstance(mService);
            db.open();
            Peer peer = db.find(device.deviceAddress);
            if (peer == null) {
                peer = new Peer(device.deviceAddress, "", new Date(), false);
                db.put(mService, peer);
            } else {
                peer.setLastContact(new Date());
                db.put(mService, peer);
            }
            if (peer.hasEid()) {
                peerDiscovered(peer);
            } else {
                allEIDs = false;
            }
        }
        // TODO bessere Bedingung finden, da EIDs geändert werden können
        if (!allEIDs) {
            mWifiP2pManager.discoverServices(mWifiP2pChannel, null);
        }
    }

    private void peerDiscovered(Peer peer) {
        Intent i = new Intent(mService, DaemonService.class);
        i.setAction(PEER_FOUND_ACTION);
        i.putExtra(EID_EXTRA, peer.getEid());
        i.putExtra(MAC_EXTRA, peer.getMacAddress());
        mService.startService(i);
        Log.d(TAG, "discovered peer:" + peer.getEid() + "," + peer.getMacAddress());
    }

    private DnsSdTxtRecordListener mRecordListener = new DnsSdTxtRecordListener() {

        @Override
        public void onDnsSdTxtRecordAvailable(String fullDomainName, Map<String, String> txtRecordMap, WifiP2pDevice srcDevice) {
            Database db = Database.getInstance(mService);
            db.open();
            Peer p = db.find(srcDevice.deviceAddress);
            if (p == null) {
                p = new Peer(srcDevice.deviceAddress, txtRecordMap.get("eid"), new Date(), false);
                db.put(mService, p);
            } else {
                p.setLastContact(new Date());
                p.setEid(txtRecordMap.get("eid"));
                db.put(mService, p);
            }
            if (p.hasEid()) {
                peerDiscovered(p);
            }
        }
    };

    private DnsSdServiceResponseListener mServiceResponseListener = new DnsSdServiceResponseListener() {
        @Override
        public void onDnsSdServiceAvailable(String instanceName, String registrationType, WifiP2pDevice srcDevice) {
            Database db = Database.getInstance(mService);
            db.open();
            Peer p = db.find(srcDevice.deviceAddress);
            if (p == null) {
                p = new Peer(srcDevice.deviceAddress, "", new Date(), false);
                db.put(mService, p);
            } else {
                srcDevice.deviceName = p.hasEid() ? p.getEid() : srcDevice.deviceName;
                p.setLastContact(new Date());
                db.put(mService, p);
            }
            if (p.hasEid()) {
                peerDiscovered(p);
            }
        }
    };
}
