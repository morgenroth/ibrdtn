package de.tubs.ibr.dtn.p2p.service;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import android.annotation.TargetApi;
import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ChannelListener;
import android.net.wifi.p2p.WifiP2pManager.DnsSdServiceResponseListener;
import android.net.wifi.p2p.WifiP2pManager.DnsSdTxtRecordListener;
import android.net.wifi.p2p.WifiP2pManager.GroupInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.net.wifi.p2p.nsd.WifiP2pDnsSdServiceInfo;
import android.net.wifi.p2p.nsd.WifiP2pDnsSdServiceRequest;
import android.util.Log;
import de.tubs.ibr.dtn.p2p.SettingsUtil;
import de.tubs.ibr.dtn.p2p.db.Database;
import de.tubs.ibr.dtn.p2p.db.Peer;
import de.tubs.ibr.dtn.service.DaemonService;

@TargetApi(16)
public class WiFiP2P4IbrDtnService extends IntentService {

    public static final String TAG = "WiFiP2P4IbrDtnService";

    public static final String START_DISCOVERY_ACTION = "de.tubs.ibr.dtn.p2p.action.START_DISCOVERY";
    public static final String STOP_DISCOVERY_ACTION = "de.tubs.ibr.dtn.p2p.action.STOP_DISCOVERY";
    public static final String PEERS_CHANGED_ACTION = "de.tubs.ibr.dtn.p2p.action.PEERS_CHANGED";
    public static final String CONNECT_TO_PEER_ACTION = "de.tubs.ibr.dtn.p2p.action.CONNECT_TO_PEER";
    public static final String DISCONNECT_FROM_PEER_ACTION = "de.tubs.ibr.dtn.p2p.action.DISCONNECT_FROM_PEER";
    public static final String CONNECTED_TO_PEER = "de.tubs.ibr.dtn.p2p.action.CONNECTED_TO_PEER";

    public static final String PEER_FOUND_ACTION = "de.tubs.ibr.dtn.p2p.action.PEER_FOUND";
    public static final String EID_EXTRA = "de.tubs.ibr.dtn.p2p.extra.EID";
    public static final String MAC_EXTRA = "de.tubs.ibr.dtn.p2p.extra.MAC";

    public static final String CONNECTION_CHANGED_ACTION = "de.tubs.ibr.dtn.p2p.action.CONNECTION_CHANGED";
    public static final String STATE_EXTRA = "de.tubs.ibr.dtn.p2p.extra.STATE";
    public static final String INTERFACE_EXTRA = "de.tubs.ibr.dtn.p2p.extra.INTERFACE";

    private WifiP2pManager wifiP2pManager;
    private Channel wifiDirectChannel;

    private WifiP2pDnsSdServiceRequest serviceRequest;

    public WiFiP2P4IbrDtnService() {
        super(TAG);
    }

    private void initializeWiFiDirect() {
        wifiP2pManager = (WifiP2pManager) getSystemService(Context.WIFI_P2P_SERVICE);

        wifiDirectChannel = wifiP2pManager.initialize(this, getMainLooper(),
                new ChannelListener() {
                    public void onChannelDisconnected() {
                        initializeWiFiDirect();
                    }
                });
    }

    @Override
    public void onCreate() {
        super.onCreate();

        initializeWiFiDirect();
        startRegistration();
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        String action = intent.getAction();

        if (START_DISCOVERY_ACTION.equals(action)) {
            discoverPeers();
        } else if (STOP_DISCOVERY_ACTION.equals(action)) {
            stopDiscovery();
        } else if (PEERS_CHANGED_ACTION.equals(action)) {
            peersChanged();
        } else if (CONNECT_TO_PEER_ACTION.equals(action)) {
            String mac = intent.getStringExtra(MAC_EXTRA);
            connectToPeer(mac);
        } else if (CONNECTED_TO_PEER.equals(action)) {
            connectedToPeer();
        }
    }

    private void connectedToPeer() {
        wifiP2pManager.requestGroupInfo(wifiDirectChannel,
                new GroupInfoListener() {
                    @Override
                    public void onGroupInfoAvailable(WifiP2pGroup group) {
                        String iface = group.getInterface();

                        Intent i = new Intent(WiFiP2P4IbrDtnService.this,
                                DaemonService.class);
                        i.setAction(CONNECTION_CHANGED_ACTION);
                        i.putExtra(STATE_EXTRA, 1);
                        i.putExtra(INTERFACE_EXTRA, iface);
                        startService(i);
                    }
                });
    }

    private void peersChanged() {
        wifiP2pManager.requestPeers(wifiDirectChannel, new PeerListListener() {
            @Override
            public void onPeersAvailable(WifiP2pDeviceList peers) {
                Log.d(TAG, "Peers available:" + peers.getDeviceList().size());
                WiFiP2P4IbrDtnService.this.checkPeers(peers);
            }
        });
    }

    private void checkPeers(WifiP2pDeviceList peers) {
        boolean allEIDs = true;
        for (WifiP2pDevice device : peers.getDeviceList()) {
            Database db = Database.getInstance(this);
            db.open();
            Peer peer = db.selectPeer(device.deviceAddress);
            if (peer == null) {
                peer = new Peer(device.deviceAddress, "", new Date(), false);
                db.insertPeer(peer);
            } else {
                peer.setLastContact(new Date());
                db.insertPeer(peer);
            }
            if (peer.hasEid()) {
                peerDiscovered(peer);
            } else {
                allEIDs = false;
            }
        }
        // TODO bessere Bedingung finden, da EIDs geändert werden können
        if (!allEIDs) {
            discoverServices();
        }
    }

    private void peerDiscovered(Peer peer) {
        Intent i = new Intent(this, DaemonService.class);
        i.setAction(PEER_FOUND_ACTION);
        i.putExtra(EID_EXTRA, peer.getEid());
        i.putExtra(MAC_EXTRA, peer.getMacAddress());
        startService(i);
        Log.d(TAG,
                "discovered peer:" + peer.getEid() + "," + peer.getMacAddress());
    }

    private void connectToPeer(String mac) {
        WifiP2pConfig config = new WifiP2pConfig();
        config.deviceAddress = mac;
        wifiP2pManager.connect(wifiDirectChannel, config, null);
    }

    private void discoverPeers() {
        wifiP2pManager.discoverPeers(wifiDirectChannel, null);
    }

    private void stopDiscovery() {
        wifiP2pManager.stopPeerDiscovery(wifiDirectChannel, null);
    }

    private void startRegistration() {
        Map<String, String> record = new HashMap<String, String>();
        record.put("eid", SettingsUtil.getEid(this));

        WifiP2pDnsSdServiceInfo serviceInfo = WifiP2pDnsSdServiceInfo
                .newInstance("_test", "_presence._tcp", record);

        wifiP2pManager.addLocalService(wifiDirectChannel, serviceInfo, null);
    }

    private void discoverServices() {
        DnsSdTxtRecordListener txtListener = new DnsSdTxtRecordListener() {

            @Override
            public void onDnsSdTxtRecordAvailable(String fullDomainName,
                    Map<String, String> txtRecordMap, WifiP2pDevice srcDevice) {
                Database db = Database.getInstance(WiFiP2P4IbrDtnService.this);
                db.open();
                Peer p = db.selectPeer(srcDevice.deviceAddress);
                if (p == null) {
                    p = new Peer(srcDevice.deviceAddress,
                            txtRecordMap.get("eid"), new Date(), false);
                    db.insertPeer(p);
                } else {
                    p.setLastContact(new Date());
                    p.setEid(txtRecordMap.get("eid"));
                    db.insertPeer(p);
                }
                if (p.hasEid()) {
                    peerDiscovered(p);
                }
            }
        };

        DnsSdServiceResponseListener srvListener = new DnsSdServiceResponseListener() {

            @Override
            public void onDnsSdServiceAvailable(String instanceName,
                    String registrationType, WifiP2pDevice srcDevice) {
                Database db = Database.getInstance(WiFiP2P4IbrDtnService.this);
                db.open();
                Peer p = db.selectPeer(srcDevice.deviceAddress);
                if (p == null) {
                    p = new Peer(srcDevice.deviceAddress, "", new Date(), false);
                    db.insertPeer(p);
                } else {
                    srcDevice.deviceName = p.hasEid() ? p.getEid()
                            : srcDevice.deviceName;
                    p.setLastContact(new Date());
                    db.insertPeer(p);
                }
                if (p.hasEid()) {
                    peerDiscovered(p);
                }
            }
        };

        wifiP2pManager.setDnsSdResponseListeners(wifiDirectChannel,
                srvListener, txtListener);

        serviceRequest = WifiP2pDnsSdServiceRequest.newInstance();
        wifiP2pManager.addServiceRequest(wifiDirectChannel, serviceRequest,
                null);
        wifiP2pManager.discoverServices(wifiDirectChannel, null);
    }
}
