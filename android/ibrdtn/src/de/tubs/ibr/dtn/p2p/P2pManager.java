package de.tubs.ibr.dtn.p2p;

import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.NetworkInfo;
import android.net.wifi.WpsInfo;
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

    private final static String TAG = "P2pManager";
    
    public static final String START_DISCOVERY_ACTION = "de.tubs.ibr.dtn.p2p.action.START_DISCOVERY";
    public static final String STOP_DISCOVERY_ACTION = "de.tubs.ibr.dtn.p2p.action.STOP_DISCOVERY";

    private DaemonService mService = null;
    
    private WifiP2pManager mWifiP2pManager = null;
    private WifiP2pManager.Channel mWifiP2pChannel = null;
    private WifiP2pDnsSdServiceRequest mServiceRequest = null;
    private WifiP2pDnsSdServiceInfo mServiceInfo = null;
    
    private Calendar mLastDisco = null;

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
    
    private WifiP2pManager.ActionListener mActionListener = new WifiP2pManager.ActionListener() {
        @Override
        public void onSuccess() {
        }
        
        @Override
        public void onFailure(int reason) {
            if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
                Log.e(TAG, "Wi-Fi Direct is not supported by this device!");
            } else {
                Log.e(TAG, "P2P: " + reason);
            }
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
        mServiceInfo = WifiP2pDnsSdServiceInfo.newInstance("DtnNode", "_dtn._tcp", record);
        
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
            mWifiP2pManager.addLocalService(mWifiP2pChannel, mServiceInfo, mActionListener);
            
            // add service discovery request
            mServiceRequest = WifiP2pDnsSdServiceRequest.newInstance();
            mWifiP2pManager.addServiceRequest(mWifiP2pChannel, mServiceRequest, mActionListener);
        }
    }
    
    public void onPause() {
        if (mWifiP2pManager != null) {
            // stop the scheduler
            Intent i = new Intent(mService, SchedulerService.class);
            i.setAction(SchedulerService.ACTION_DEACTIVATE_SCHEDULER);
            mService.startService(i);
            
            // cancel all connection request
            mWifiP2pManager.cancelConnect(mWifiP2pChannel, mActionListener);
            
            // clear all service discovery requests
            mWifiP2pManager.clearServiceRequests(mWifiP2pChannel, mActionListener);
            
            // remove local service description
            mWifiP2pManager.clearLocalServices(mWifiP2pChannel, mActionListener);
            
            // stop all peer discoveries
            mWifiP2pManager.stopPeerDiscovery(mWifiP2pChannel, mActionListener);
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
        //Log.d(TAG, "found peer: " + eid + ", " + identifier);
        super.fireDiscovered(new EID(eid), identifier, 90, 10);
    }

    @Override
    public void connect(String data) {
        // get the peer object
        Peer p = Database.getInstance(mService).find(data);
        
        // stop here, if there is no peer object
        if (p == null) return;
        
        // stop here, if we already tried to connect within the last minute
        if (p.getConnectState() > 1) return;
        
        // set the connection state to 2 (connecting)
        p.setConnectState(2);
        Database.getInstance(mService).updateState(p);
        
        // connect to the peer identified by "data"
        WifiP2pConfig config = new WifiP2pConfig();
        config.deviceAddress = data;
        config.groupOwnerIntent = -1;
        config.wps.setup = WpsInfo.PBC;
        mWifiP2pManager.connect(mWifiP2pChannel, config, mActionListener);
        
        Log.i(TAG, "connect to " + p.toString());
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
                mWifiP2pManager.discoverPeers(mWifiP2pChannel, mActionListener);
            } else if (STOP_DISCOVERY_ACTION.equals(action)) {
                mWifiP2pManager.stopPeerDiscovery(mWifiP2pChannel, mActionListener);
            }
        }
        
        private void connectionChanged(Context context, Intent intent) {
            NetworkInfo netInfo = intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
            Log.d(TAG, "DetailedState: " + netInfo.getDetailedState());
            
            if (netInfo.isConnected()) {
                // request group info to get the interface of the group
                mWifiP2pManager.requestGroupInfo(mWifiP2pChannel, mGroupInfoListener);
            }
        }

        private void peersChanged(Context context, Intent intent) {
            mWifiP2pManager.requestPeers(mWifiP2pChannel, mPeerListListener);
        }

        private void discoveryChanged(Context context, Intent intent) {
            int discoveryState = intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE, WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED);
            if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED) {
                Intent i = new Intent(context, SchedulerService.class);
                i.setAction(SchedulerService.ACTION_CHECK_STATE);
                context.startService(i);
            } else {
            }
        }
    };

    private void peerDiscovered(Peer peer) {
        //Log.d(TAG, "discovered peer:" + peer.getEndpoint() + "," + peer.getP2pAddress());
        fireDiscovered(peer.getEndpoint(), peer.getP2pAddress());
    }
    
    /**
     * This listener is used to get the interface of new groups
     */
    private GroupInfoListener mGroupInfoListener = new GroupInfoListener() {
        @Override
        public void onGroupInfoAvailable(WifiP2pGroup group) {
            String iface = group.getInterface();
            
            // add the interface
            fireInterfaceUp(iface);
        }
    };
    
    private PeerListListener mPeerListListener = new PeerListListener() {
        @Override
        public void onPeersAvailable(WifiP2pDeviceList peers) {
            // Log.d(TAG, "Peers available:" + peers.getDeviceList().size());
            
            boolean allEIDs = true;
            for (WifiP2pDevice device : peers.getDeviceList()) {
                Database db = Database.getInstance(mService);
                db.open();
                Peer peer = db.find(device.deviceAddress);
                if (peer == null) {
                    peer = new Peer(device.deviceAddress);
                    db.put(peer);
                } else {
                    peer.touch();
                    db.put(peer);
                }
                if (peer.hasEndpoint()) {
                    peerDiscovered(peer);
                } else {
                    allEIDs = false;
                }
            }
            // TODO bessere Bedingung finden, da EIDs geändert werden können
            if (!allEIDs) {
                requestServiceDiscovery();
            }
        }
    };
    
    private void requestServiceDiscovery() {
        if (mLastDisco != null) {
            if (mLastDisco.after(Calendar.getInstance())) return;
        }
        
        // only once per minute
        mLastDisco = Calendar.getInstance();
        mLastDisco.add(Calendar.MINUTE, 1);
        
        // start service discovery
        mWifiP2pManager.discoverServices(mWifiP2pChannel, mActionListener);
    }

    private DnsSdTxtRecordListener mRecordListener = new DnsSdTxtRecordListener() {

        @Override
        public void onDnsSdTxtRecordAvailable(String fullDomainName, Map<String, String> txtRecordMap, WifiP2pDevice srcDevice) {
            Log.d(TAG, "DnsSdTxtRecord discovered: " + fullDomainName + ", " + txtRecordMap.toString());
            
            Database db = Database.getInstance(mService);
            db.open();
            Peer p = db.find(srcDevice.deviceAddress);
            if (p == null) {
                p = new Peer(srcDevice.deviceAddress);
                p.setEndpoint(txtRecordMap.get("eid"));
                db.put(p);
            } else {
                p.touch();
                p.setEndpoint(txtRecordMap.get("eid"));
                db.put(p);
            }
            if (p.hasEndpoint()) {
                peerDiscovered(p);
            }
        }
    };

    private DnsSdServiceResponseListener mServiceResponseListener = new DnsSdServiceResponseListener() {
        @Override
        public void onDnsSdServiceAvailable(String instanceName, String registrationType, WifiP2pDevice srcDevice) {
            Log.d(TAG, "SdService discovered: " + instanceName + ", " + registrationType);
            
            Database db = Database.getInstance(mService);
            db.open();
            Peer p = db.find(srcDevice.deviceAddress);
            if (p == null) {
                p = new Peer(srcDevice.deviceAddress);
                db.put(p);
            } else {
                srcDevice.deviceName = p.hasEndpoint() ? p.getEndpoint() : srcDevice.deviceName;
                p.touch();
                db.put(p);
            }
            if (p.hasEndpoint()) {
                peerDiscovered(p);
            }
        }
    };
}
