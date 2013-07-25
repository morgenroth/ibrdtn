package de.tubs.ibr.dtn.p2p;

import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;

import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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
    
    private enum ManagerState {
        UNINITIALIZED,
        INITIALIZED,
        PENDING,
        ENABLED,
        DISABLED,
        NOT_SUPPORTED
    };
    
    private ManagerState mManagerState = ManagerState.UNINITIALIZED;

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
    
    private WifiP2pManager.ChannelListener mChannelListener = new WifiP2pManager.ChannelListener() {
        @Override
        public void onChannelDisconnected() {
            synchronized(P2pManager.this) {
                onPause();
                onDestroy();
                mManagerState = ManagerState.NOT_SUPPORTED;
            }
        }
    };
    
    public synchronized void create() {
        if (ManagerState.NOT_SUPPORTED.equals(mManagerState)) return;
        onCreate();
    }
    
    public synchronized void destroy() {
        if (ManagerState.NOT_SUPPORTED.equals(mManagerState)) return;
        onDestroy();
    }
    
    public synchronized void pause() {
        if (ManagerState.NOT_SUPPORTED.equals(mManagerState)) return;
        if (ManagerState.ENABLED.equals(mManagerState)) {
            onP2pDisabled();
        }
        onPause();
    }
    
    public synchronized void resume() {
        if (ManagerState.NOT_SUPPORTED.equals(mManagerState)) return;
        onResume();
    }

    private synchronized void onCreate() {
        // daemon is up
        Log.d(TAG, "onCreate() state: " + mManagerState.toString());
        
        // get the WifiP2pManager
        mWifiP2pManager = (WifiP2pManager) mService.getSystemService(Context.WIFI_P2P_SERVICE);
        
        // set the current state
        mManagerState = ManagerState.INITIALIZED;
    }
    
    private synchronized void onResume() {
        Log.d(TAG, "onResume() state: " + mManagerState.toString());
        
        // transition only from INITIALIZED state
        if (!ManagerState.INITIALIZED.equals(mManagerState)) return;
        
        // listen to wifi p2p events
        IntentFilter p2p_filter = new IntentFilter();
        p2p_filter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        p2p_filter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        p2p_filter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        p2p_filter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
        p2p_filter.addAction(START_DISCOVERY_ACTION);
        p2p_filter.addAction(STOP_DISCOVERY_ACTION);
        mService.registerReceiver(mP2pEventReceiver, p2p_filter);
        
        // set the current state
        mManagerState = ManagerState.PENDING;
        
        // create a new Wi-Fi P2P channel
        mWifiP2pChannel = mWifiP2pManager.initialize(mService, mService.getMainLooper(), mChannelListener);
    }
    
    /**
     * This method is called if the P2p has been initialized
     */
    private synchronized void onP2pEnabled() {
        Log.d(TAG, "onP2pEnabled() state: " + mManagerState.toString());
        
        // transition only from PENDING or DISABLED state
        if (!(ManagerState.PENDING.equals(mManagerState) || ManagerState.DISABLED.equals(mManagerState))) return;
        
        // initialize service discovery mechanisms
        initializeServiceDiscovery();
        
        // start the scheduler
        Intent i = new Intent(mService, SchedulerService.class);
        i.setAction(SchedulerService.ACTION_ACTIVATE_SCHEDULER);
        mService.startService(i);
        
        // set the current state
        mManagerState = ManagerState.ENABLED;
    }
    
    private synchronized void onP2pDisabled() {
        Log.d(TAG, "onP2pDisabled() state: " + mManagerState.toString());
        
        // stop the scheduler
        Intent i = new Intent(mService, SchedulerService.class);
        i.setAction(SchedulerService.ACTION_DEACTIVATE_SCHEDULER);
        mService.startService(i);
        
        if (ManagerState.ENABLED.equals(mManagerState)) {
            // cancel all connection request
            mWifiP2pManager.cancelConnect(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
                @Override
                public void onFailure(int reason) {
                    Log.e(TAG, "cancellation of all connections failed: " + reason);
                }

                @Override
                public void onSuccess() {
                    Log.d(TAG, "all connects cancelled");
                }
            });
            
            // terminate service discovery mechanisms
            terminateServiceDiscovery();
        }
        
        // set the current state
        mManagerState = ManagerState.DISABLED;
    }
    
    private synchronized void onPause() {
        Log.d(TAG, "onPause() state: " + mManagerState.toString());
        
        // stop listening to wifi p2p events
        mService.unregisterReceiver(mP2pEventReceiver);
        
        // set the current state
        mManagerState = ManagerState.INITIALIZED;
    }

    private synchronized void onDestroy() {
        // daemon goes down
        mLastDisco = null;
        mServiceInfo = null;
        mServiceRequest = null;
        mWifiP2pChannel = null;
        mWifiP2pManager = null;
        
        // set the current state
        mManagerState = ManagerState.UNINITIALIZED;
    }
    
    private void initializeServiceDiscovery() {
        // set service discovery listener
        mWifiP2pManager.setDnsSdResponseListeners(mWifiP2pChannel, mServiceResponseListener, mRecordListener);
        
        // create local service info
        Map<String, String> record = new HashMap<String, String>();
        record.put("eid", SettingsUtil.getEid(mService));
        mServiceInfo = WifiP2pDnsSdServiceInfo.newInstance("DtnNode", "_dtn._tcp", record);
        
        // add local service description
        mWifiP2pManager.addLocalService(mWifiP2pChannel, mServiceInfo, new WifiP2pManager.ActionListener() {
            @Override
            public void onFailure(int reason) {
                if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
                    Log.e(TAG, "failed to add local service: Wi-Fi Direct is not supported by this device!");
                } else {
                    Log.e(TAG, "failed to add local service: " + reason);
                }
            }

            @Override
            public void onSuccess() {
                Log.d(TAG, "local service added");
            }
        });
        
        // add service discovery request
        mServiceRequest = WifiP2pDnsSdServiceRequest.newInstance();
        mWifiP2pManager.addServiceRequest(mWifiP2pChannel, mServiceRequest, new WifiP2pManager.ActionListener() {
            @Override
            public void onFailure(int reason) {
                if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
                    Log.e(TAG, "failed to add service request: Wi-Fi Direct is not supported by this device!");
                } else {
                    Log.e(TAG, "failed to add service request: " + reason);
                }
            }

            @Override
            public void onSuccess() {
                Log.d(TAG, "service request added");
            }
        });
    }
    
    private void terminateServiceDiscovery() {
        // clear all service discovery requests
        mWifiP2pManager.clearServiceRequests(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
            @Override
            public void onFailure(int reason) {
                Log.e(TAG, "cancellation of all service requests failed: " + reason);
            }

            @Override
            public void onSuccess() {
                Log.d(TAG, "all service request cleared");
            }
        });
        
        // remove local service description
        mWifiP2pManager.clearLocalServices(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
            @Override
            public void onFailure(int reason) {
                Log.e(TAG, "clearing of all local services failed: " + reason);
            }

            @Override
            public void onSuccess() {
                Log.d(TAG, "all local services cleared");
            }
        });
        
        // stop all peer discoveries
        mWifiP2pManager.stopPeerDiscovery(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
            @Override
            public void onFailure(int reason) {
                Log.e(TAG, "stop peer discovery failed: " + reason);
            }

            @Override
            public void onSuccess() {
                Log.d(TAG, "peer discovery stopped");
            }
        });
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
        if (mWifiP2pChannel == null) return;
        
        // get the peer object
        final Peer p = Database.getInstance(mService).find(data);
        
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
        
        Log.d(TAG, "connect to " + p.toString());
        
        mWifiP2pManager.connect(mWifiP2pChannel, config, new WifiP2pManager.ActionListener() {
            @Override
            public void onFailure(int reason) {
                Log.e(TAG, "connection to " + p.toString() + " failed: " + reason);
            }

            @Override
            public void onSuccess() {
                Log.d(TAG, "connection in progress to " + p.toString());
            }
        });
    }

    @Override
    public void disconnect(String data) {
        if (mWifiP2pChannel == null) return;
        
        // disconnect from the peer identified by "data"
        Log.i(TAG, "disconnect request: " + data);
    }
    
    public BroadcastReceiver mP2pEventReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            // do nothing if the stack isn't initialized
            if (mWifiP2pManager == null) return;
            
            String action = intent.getAction();

            if (WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
                discoveryChanged(context, intent);
            } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
                peersChanged(context, intent);
            } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
                connectionChanged(context, intent);
            } else if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
                stateChanged(context, intent);
            } else if (START_DISCOVERY_ACTION.equals(action)) {
                startDiscovery(context, intent);
            } else if (STOP_DISCOVERY_ACTION.equals(action)) {
                stopDiscovery(context, intent);
            }
        }
        
        private void connectionChanged(Context context, Intent intent) {
            //WifiP2pInfo p2pInfo = intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_INFO);
            NetworkInfo netInfo = intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);
            
            // available with Android 4.3
            // WifiP2pGroup p2pGroup = intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_GROUP);
            // fireInterfaceDown( p2pGroup.getInterface() );
            
            if (netInfo.isConnected()) {
                // request group info to get the interface of the group
                mWifiP2pManager.requestGroupInfo(mWifiP2pChannel, new GroupInfoListener() {
                    @Override
                    public void onGroupInfoAvailable(WifiP2pGroup group) {
                        String iface = group.getInterface();
                        
                        // add the interface
                        fireInterfaceUp(iface);
                    }
                });
            }
        }

        private void peersChanged(Context context, Intent intent) {
            mWifiP2pManager.requestPeers(mWifiP2pChannel, mPeerListListener);
        }

        private void discoveryChanged(Context context, Intent intent) {
            int discoveryState = intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE, WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED);
            
            if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED) {
                Log.d(TAG, "Peer discovery has been stopped");
                
                // check scheduler state if P2P is enabled
                if (ManagerState.ENABLED.equals(mManagerState)) {
                    Intent i = new Intent(context, SchedulerService.class);
                    i.setAction(SchedulerService.ACTION_CHECK_STATE);
                    context.startService(i);
                }
            }
            else if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STARTED) {
                Log.d(TAG, "Peer discovery has been started");
            }
            
        }
        
        private void stateChanged(Context context, Intent intent) {
            int p2pState = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, WifiP2pManager.WIFI_P2P_STATE_DISABLED);
            
            if (p2pState == WifiP2pManager.WIFI_P2P_STATE_DISABLED) {
                Log.d(TAG, "Wi-Fi P2P has been disabled.");
                onP2pDisabled();
            }
            else if (p2pState == WifiP2pManager.WIFI_P2P_STATE_ENABLED) {
                Log.d(TAG, "Wi-Fi P2P has been enabled.");
                onP2pEnabled();
            }
        }
        
        private void startDiscovery(Context context, Intent intent) {
            if (!ManagerState.ENABLED.equals(mManagerState)) return;

            mWifiP2pManager.discoverPeers(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
                @Override
                public void onFailure(int reason) {
                    Log.e(TAG, "starting peer discovery failed: " + reason);
                }

                @Override
                public void onSuccess() {
                    Log.d(TAG, "peer discovery started");
                }
            });
        }
        
        private void stopDiscovery(Context context, Intent intent) {
            mWifiP2pManager.stopPeerDiscovery(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
                @Override
                public void onFailure(int reason) {
                    Log.e(TAG, "stop peer discovery failed: " + reason);
                }

                @Override
                public void onSuccess() {
                    Log.d(TAG, "peer discovery stopped");
                }
            });
        }
    };

    private void peerDiscovered(Peer peer) {
        //Log.d(TAG, "discovered peer:" + peer.getEndpoint() + "," + peer.getP2pAddress());
        fireDiscovered(peer.getEndpoint(), peer.getP2pAddress());
    }
    
    private PeerListListener mPeerListListener = new PeerListListener() {
        @Override
        public void onPeersAvailable(WifiP2pDeviceList peers) {
            // Log.d(TAG, "Peers available:" + peers.getDeviceList().size());
            
            boolean allEidsValid = true;
            for (WifiP2pDevice device : peers.getDeviceList()) {
                Database db = Database.getInstance(mService);
                db.open();
                Peer peer = db.find(device.deviceAddress);
                
                if (peer == null) {
                    peer = new Peer(device.deviceAddress);
                } else {
                    peer.touch();
                }
                
                db.put(peer);
                
                if (peer.hasEndpoint() && peer.isEndpointValid()) {
                    peerDiscovered(peer);
                } else {
                    allEidsValid = false;
                }
            }

            if (!allEidsValid) {
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
        mWifiP2pManager.discoverServices(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
            @Override
            public void onFailure(int reason) {
                Log.e(TAG, "starting service discovery failed: " + reason);
            }

            @Override
            public void onSuccess() {
                Log.d(TAG, "service discovery started");
            }
        });
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
            } else {
                p.touch();
            }
            
            p.setEndpoint(txtRecordMap.get("eid"));
            db.put(p);
            
            peerDiscovered(p);
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
            } else {
                p.touch();
            }
            
            db.put(p);
            
            if (p.hasEndpoint()) {
                peerDiscovered(p);
            }
        }
    };
}
