
package de.tubs.ibr.dtn.service;

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import android.annotation.SuppressLint;
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
import android.os.Build;
import android.util.Log;
import de.tubs.ibr.dtn.daemon.Preferences;
import de.tubs.ibr.dtn.swig.EID;
import de.tubs.ibr.dtn.swig.NativeP2pManager;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class P2pManager extends NativeP2pManager {

	private final static String TAG = "P2pManager";

	public static final String INTENT_P2P_STATE_CHANGED = "de.tubs.ibr.dtn.p2p.action.STATE_CHANGED";
	public static final String EXTRA_P2P_STATE = "de.tubs.ibr.dtn.p2p.action.EXTRA_STATE";

	private DaemonService mService = null;

	private WifiP2pManager mWifiP2pManager = null;
	private WifiP2pManager.Channel mWifiP2pChannel = null;
	private WifiP2pDnsSdServiceRequest mServiceRequest = null;
	private WifiP2pDnsSdServiceInfo mServiceInfo = null;

	private boolean mServiceEnabled = false;
	private boolean mDiscoveryEnabled = false;

	/**
	 * The p2p interfaces are stored in this set. Every time the P2pManager goes
	 * into pause all interfaces are signaled as down. On resume the interfaces
	 * are signaled as active again.
	 */
	private HashSet<String> mP2pInterfaces = new HashSet<String>();

	private enum ManagerState {
		UNINITIALIZED,
		INACTIVE,
		ACTIVE
	};

	private ManagerState mManagerState = ManagerState.UNINITIALIZED;

	/**
	 * How to control the P2P API of the daemon The following control calls are
	 * only possible if the client is connected to the daemon. That means, do
	 * such a call only if "initialize" was successful and "destroy" has not
	 * been called yet. Call a "fireDiscovered" event for each discovered peer
	 * earlier than every 120 seconds. fireDiscovered(new
	 * ibrdtn.api.object.SingletonEndpoint("dtn://test-node"), data); If a new
	 * interface has been created due to a P2P connection, then call the
	 * "fireInterfaceUp" event. fireInterfaceUp("wlan0-p2p0"); If a P2P
	 * interface goes down then call "fireInterfaceDown".
	 * fireInterfaceDown("wlan0-p2p0");
	 */

	public P2pManager(DaemonService service) {
		super("P2P:WIFI");
		this.mService = service;
	}

	private WifiP2pManager.ChannelListener mChannelListener = new WifiP2pManager.ChannelListener() {
		@Override
		public void onChannelDisconnected() {
			Log.d(TAG, "disconnected");
		}
	};

	public synchronized boolean isActive() {
		return ManagerState.ACTIVE.equals(mManagerState);
	}

	public synchronized void setEnabled(boolean enabled) {
		// allow P2P scanning
		mServiceEnabled = enabled;

		if (enabled && mDiscoveryEnabled) {
			// start discovery on enable if discovery is active
			startDiscovery();
		}
		else if (!enabled && mDiscoveryEnabled) {
			// stop discovery on disable
			stopDiscovery();
			mDiscoveryEnabled = true;
		}
	}

	private void setState(ManagerState state) {
		// new state debug
		Log.d(TAG, "state transition: " + mManagerState.toString() + " -> " + state.toString());
		
		// change state
		mManagerState = state;

		// generate intent
		Intent i = new Intent(INTENT_P2P_STATE_CHANGED);
		i.putExtra(EXTRA_P2P_STATE, ManagerState.ACTIVE.equals(state));
		mService.sendBroadcast(i);
	}

	public synchronized void create() {
		// allowed transition from UNINITIALIZED
		if (ManagerState.UNINITIALIZED.equals(mManagerState)) {
			onCreate();
		}
	}

	public synchronized void destroy() {
		if (ManagerState.ACTIVE.equals(mManagerState)) {
			onP2pDisabled();
		}
		
		// allowed transition from ACTIVE and INACTIVE
		if (ManagerState.INACTIVE.equals(mManagerState)) {
			onDestroy();
		}
	}

	private synchronized void onCreate() {
		// listen to wifi p2p events
		IntentFilter p2p_filter = new IntentFilter();
		p2p_filter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
		p2p_filter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
		p2p_filter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
		p2p_filter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);

		// register to P2p related intents
		mService.registerReceiver(mP2pEventReceiver, p2p_filter);

		// get the WifiP2pManager
		mWifiP2pManager = (WifiP2pManager) mService.getSystemService(Context.WIFI_P2P_SERVICE);

		// set the current state
		setState(ManagerState.INACTIVE);

		// create a new Wi-Fi P2P channel
		mWifiP2pChannel = mWifiP2pManager.initialize(mService, mService.getMainLooper(),
				mChannelListener);
	}

	/**
	 * This method is called if the P2p has been initialized
	 */
	private synchronized void onP2pEnabled() {
		// transition only from INACTIVE state
		if (!ManagerState.INACTIVE.equals(mManagerState)) return;

		// initialize service discovery mechanisms
		initializeServiceDiscovery();

		// remove all inactive interfaces
		HashSet<String> activeInterfaces = getActiveInterfaces();
		mP2pInterfaces.retainAll(activeInterfaces);

		// bring up all p2p interfaces
		for (String iface : mP2pInterfaces) {
			Log.d(TAG, "restore connection on interface " + iface);
			fireInterfaceUp(iface);
		}

		// set the current state
		setState(ManagerState.ACTIVE);
		
		// restart discovery
		if (mDiscoveryEnabled) {
			startDiscovery();
		}
	}

	private synchronized void onP2pDisabled() {
		// transition only from ACTIVE state
		if (!ManagerState.ACTIVE.equals(mManagerState)) return;
		
		// bring down all p2p interfaces
		for (String iface : mP2pInterfaces) {
			Log.d(TAG, "disable connection on interface " + iface);

			fireInterfaceDown(iface);
		}

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

		// set the current state
		setState(ManagerState.INACTIVE);
	}

	private synchronized void onDestroy() {
		// stop listening to wifi p2p events
		mService.unregisterReceiver(mP2pEventReceiver);

		// daemon goes down
		mServiceInfo = null;
		mServiceRequest = null;
		mWifiP2pChannel = null;
		mWifiP2pManager = null;

		// set the current state
		setState(ManagerState.UNINITIALIZED);
	}

	private void initializeServiceDiscovery() {
		// set service discovery listener
		mWifiP2pManager.setDnsSdResponseListeners(mWifiP2pChannel, mServiceResponseListener, mRecordListener);

		// create local service info
		Map<String, String> record = new HashMap<String, String>();
		record.put("eid", Preferences.getEndpoint(mService));
		
		// create service info
		mServiceInfo = WifiP2pDnsSdServiceInfo.newInstance("DtnNode", "_dtn._tcp", record);
		
		// create service request
		mServiceRequest = WifiP2pDnsSdServiceRequest.newInstance("DtnNode", "_dtn._tcp");

		// add local service description
		mWifiP2pManager.addLocalService(mWifiP2pChannel, mServiceInfo,
				new WifiP2pManager.ActionListener() {
					@Override
					public void onFailure(int reason) {
						if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
							Log.e(TAG,
									"failed to add local service: Wi-Fi Direct is not supported by this device!");
						} else {
							Log.e(TAG, "failed to add local service: " + reason);
						}
					}

					@Override
					public void onSuccess() {
						Log.d(TAG, "local service added");
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

	@Override
	public void connect(final String data) {
		// check if service is enabled
		if (!mServiceEnabled) return;
		
		// check if stack is active
		if (!ManagerState.ACTIVE.equals(mManagerState)) return;

		// connect to the peer identified by "data"
		WifiP2pConfig config = new WifiP2pConfig();
		config.deviceAddress = data;
		config.groupOwnerIntent = -1;
		config.wps.setup = WpsInfo.PBC;

		Log.d(TAG, "connect to " + data);

		mWifiP2pManager.connect(mWifiP2pChannel, config, new WifiP2pManager.ActionListener() {
			@Override
			public void onFailure(int reason) {
				Log.e(TAG, "connection to " + data + " failed: " + reason);
			}

			@Override
			public void onSuccess() {
				Log.d(TAG, "connection in progress to " + data);
			}
		});
	}

	@Override
	public void disconnect(String data) {
		// check if stack is active
		if (!ManagerState.ACTIVE.equals(mManagerState)) return;

		// disconnect from the peer identified by "data"
		Log.i(TAG, "disconnect request: " + data);
	}

	private BroadcastReceiver mP2pEventReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG, "P2P action: " + intent);

			String action = intent.getAction();

			if (WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
				discoveryChanged(intent);
			} else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
				peersChanged();
			} else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
				connectionChanged(intent);
			} else if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
				stateChanged(intent);
			}
		}

		@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
		private void connectionChanged(Intent intent) {
			// WifiP2pInfo p2pInfo =
			// intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_INFO);
			NetworkInfo netInfo = intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);

			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
				// available since Android 4.3
				WifiP2pGroup p2pGroup = intent
						.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_GROUP);
				String iface = p2pGroup.getInterface();

				Log.d(TAG, "connection changed " + netInfo.toString() + " | " + p2pGroup.toString());

				if (netInfo.isConnected() && (iface != null)) {
					Log.d(TAG, "connection up " + iface);
					mP2pInterfaces.add(iface);
					fireInterfaceUp(iface);
				} else if (mP2pInterfaces.contains(iface)) {
					Log.d(TAG, "connection down " + iface);
					mP2pInterfaces.remove(iface);
					fireInterfaceDown(iface);
				}
			} else if (netInfo.isConnected()) {
				// request group info to get the interface of the group
				mWifiP2pManager.requestGroupInfo(mWifiP2pChannel, new GroupInfoListener() {
					@Override
					public void onGroupInfoAvailable(WifiP2pGroup group) {
						String iface = group.getInterface();

						// add the interface to the interface set
						mP2pInterfaces.add(iface);

						// add the interface
						fireInterfaceUp(iface);
					}
				});
			}
		}

		private void peersChanged() {
			// check if service is enabled
			if (!mServiceEnabled) return;

			// request peer list
			mWifiP2pManager.requestPeers(mWifiP2pChannel, mPeerListListener);
		}

		private void discoveryChanged(Intent intent) {
			int discoveryState = intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE,
					WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED);

			if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED) {
				Log.d(TAG, "Peer discovery has been stopped");
			}
			else if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STARTED) {
				Log.d(TAG, "Peer discovery has been started");
			}
		}

		private void stateChanged(Intent intent) {
			int p2pState = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE,
					WifiP2pManager.WIFI_P2P_STATE_DISABLED);

			if (p2pState == WifiP2pManager.WIFI_P2P_STATE_DISABLED) {
				Log.d(TAG, "Wi-Fi P2P has been disabled.");
				onP2pDisabled();
			}
			else if (p2pState == WifiP2pManager.WIFI_P2P_STATE_ENABLED) {
				Log.d(TAG, "Wi-Fi P2P has been enabled.");
				onP2pEnabled();
			}
		}
	};

	@SuppressLint("NewApi")
	public synchronized void startDiscovery() {
		// set disco marker
		mDiscoveryEnabled = true;
		
		// check if service is enabled
		if (!mServiceEnabled) return;

		// check if stack is active
		if (!ManagerState.ACTIVE.equals(mManagerState)) return;
		
		// remove previous service request
		mWifiP2pManager.removeServiceRequest(mWifiP2pChannel, mServiceRequest, new WifiP2pManager.ActionListener() {
			@Override
			public void onFailure(int reason) {
				if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
					Log.e(TAG,
							"failed to add service request: Wi-Fi Direct is not supported by this device!");
				} else {
					Log.e(TAG, "failed to add service request: " + reason);
				}
			}

			@Override
			public void onSuccess() {
				Log.d(TAG, "service request removed");
			}
		});

		// add service discovery request
		mWifiP2pManager.addServiceRequest(mWifiP2pChannel, mServiceRequest, new WifiP2pManager.ActionListener() {
			@Override
			public void onFailure(int reason) {
				if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
					Log.e(TAG,
							"failed to add service request: Wi-Fi Direct is not supported by this device!");
				} else {
					Log.e(TAG, "failed to add service request: " + reason);
				}
			}

			@Override
			public void onSuccess() {
				Log.d(TAG, "service request added");
			}
		});

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

	public synchronized void stopDiscovery() {
		// set disco marker
		mDiscoveryEnabled = false;
		
		// check if stack is active
		if (!ManagerState.ACTIVE.equals(mManagerState)) return;

		// stop ongoing discovery
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

	private PeerListListener mPeerListListener = new PeerListListener() {
		@Override
		public void onPeersAvailable(WifiP2pDeviceList peers) {
			Log.d(TAG, "Peers available: " + peers.getDeviceList().size());

			for (WifiP2pDevice device : peers.getDeviceList()) {
				Log.d(TAG, "Peer: " + device.deviceName + " (" + device.deviceAddress + ")");
			}
		}
	};

	private DnsSdTxtRecordListener mRecordListener = new DnsSdTxtRecordListener() {
		@Override
		public void onDnsSdTxtRecordAvailable(String fullDomainName,
				Map<String, String> txtRecordMap, WifiP2pDevice srcDevice) {
			Log.d(TAG,
					"DnsSdTxtRecord discovered: " + fullDomainName + ", "
							+ txtRecordMap.toString());
			
			fireDiscovered(new EID(txtRecordMap.get("eid")), srcDevice.deviceAddress, 90, 10);
		}
	};

	private DnsSdServiceResponseListener mServiceResponseListener = new DnsSdServiceResponseListener() {
		@Override
		public void onDnsSdServiceAvailable(String instanceName, String registrationType,
				WifiP2pDevice srcDevice) {
			Log.d(TAG, "SdService discovered: " + instanceName + ", " + registrationType);
		}
	};

	private HashSet<String> getActiveInterfaces() {
		HashSet<String> ret = new HashSet<String>();

		try {
			ArrayList<NetworkInterface> ilist = Collections.list(NetworkInterface
					.getNetworkInterfaces());

			// scan for known network devices
			for (NetworkInterface i : ilist)
			{
				String iface = i.getDisplayName();
				ret.add(iface);
			}
		} catch (SocketException e) {
			Log.e(TAG, "failed to get active interfaces", e);
		}

		return ret;
	}
}
