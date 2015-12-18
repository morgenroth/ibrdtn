
package de.tubs.ibr.dtn.service;

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import android.annotation.SuppressLint;
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

	private boolean mDiscoveryEnabled = false;
	private IntentFilter mP2pFilter = null;
	private boolean mResumed = false;
	
	private class P2pDevice {
		public WifiP2pDevice device = null;
		public EID endpoint = null;
		public boolean pending = false;
	};

	/**
	 * The p2p interfaces are stored in this set. Every time the P2pManager goes
	 * into pause all interfaces are signaled as down. On resume the interfaces
	 * are signaled as active again.
	 */
	private HashSet<String> mInterfaces = new HashSet<String>();
	
	private HashMap<String, P2pDevice> mDevices = new HashMap<String, P2pDevice>();

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

	public boolean isSupported() {
		return mWifiP2pManager != null;
	}
	
	SharedPreferences.OnSharedPreferenceChangeListener mPrefListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
		@Override
		public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
			if (Preferences.KEY_P2P_ENABLED.equals(key)) {
				if (prefs.getBoolean(key, false) && mResumed) {
					resume();
				} else if (mResumed) {
					pause();
				}
			}
		}
	};
	
	private WifiP2pManager.ChannelListener mChannelListener = new WifiP2pManager.ChannelListener() {
		@Override
		public void onChannelDisconnected() {
			Log.e(TAG, "Channel disconnected.");
		}
	};

	public synchronized void onCreate() {
		// listen to wifi p2p events
		mP2pFilter = new IntentFilter();
		mP2pFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
		mP2pFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
		mP2pFilter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
		mP2pFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
		
		// create local service info
		Map<String, String> record = new HashMap<String, String>();
		SharedPreferences prefs = mService.getSharedPreferences("dtnd", Context.MODE_PRIVATE);
		record.put("eid", Preferences.getEndpoint(mService, prefs));
		
		// create service info
		mServiceInfo = WifiP2pDnsSdServiceInfo.newInstance("DtnNode", "_dtn._tcp", record);
		
		// create service request
		mServiceRequest = WifiP2pDnsSdServiceRequest.newInstance("DtnNode", "_dtn._tcp");

		// get the WifiP2pManager
		mWifiP2pManager = (WifiP2pManager) mService.getSystemService(Context.WIFI_P2P_SERVICE);
		
		// listen to preference changes
		prefs.registerOnSharedPreferenceChangeListener(mPrefListener);
		
		// notify GUI if P2P is supported
		Intent i = new Intent(INTENT_P2P_STATE_CHANGED);
		i.putExtra(EXTRA_P2P_STATE, mWifiP2pManager != null);
		mService.sendBroadcast(i);
		
		// restore resumed state
		if (mResumed && prefs.getBoolean(Preferences.KEY_P2P_ENABLED, false)) resume();
		
		// register to P2p related intents
		mService.registerReceiver(mP2pEventReceiver, mP2pFilter);
	}
	
	public synchronized void onResume() {
		mResumed = true;
		
		SharedPreferences prefs = mService.getSharedPreferences("dtnd", Context.MODE_PRIVATE);
		if (!prefs.getBoolean(Preferences.KEY_P2P_ENABLED, false)) return;
		
		resume();
	}
	
	private synchronized void resume() {
		if (mWifiP2pChannel != null) return;
		
		if (mWifiP2pManager != null) {
			// create a new Wi-Fi P2P channel
			mWifiP2pChannel = mWifiP2pManager.initialize(mService, mService.getMainLooper(), mChannelListener);
			
			if (mWifiP2pChannel == null) {
				Log.e(TAG, "Can not allocate a P2P channel");
			} else {
				// set service discovery listener
				mWifiP2pManager.setDnsSdResponseListeners(mWifiP2pChannel, mServiceResponseListener, mRecordListener);
				
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
				
				// remove all inactive interfaces
				HashSet<String> activeInterfaces = getActiveInterfaces();
				mInterfaces.retainAll(activeInterfaces);

				// bring up all p2p interfaces
				for (String iface : mInterfaces) {
					Log.d(TAG, "restore connection on interface " + iface);
					fireInterfaceUp(iface);
				}
				
				// restart discovery
				if (mDiscoveryEnabled) {
					startDiscovery();
				}
			}
		} else {
			Log.e(TAG, "Wi-Fi P2P Service is null");
		}
	}
	
	public synchronized void onPause() {
		if (!mResumed) return;
		mResumed = false;
		pause();
	}
	
	private synchronized void pause() {
		// only proceed if the channel is assigned
		if (mWifiP2pChannel == null) return;
		
		// bring down all p2p interfaces
		for (String iface : mInterfaces) {
			Log.d(TAG, "disable connection on interface " + iface);
			fireInterfaceDown(iface);
		}

		// disconnect all groups
		mWifiP2pManager.removeGroup(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
			@Override
			public void onSuccess() {
				Log.e(TAG, "group removed");
			}

			@Override
			public void onFailure(int reason) {
				Log.e(TAG, "can not remove group: " + reason);
			}
		});
		
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
		
		// release channel
		mWifiP2pChannel = null;
	}

	public synchronized void onDestroy() {
		// switch to pause if was in resumed state
		if (mResumed) pause();
		
		// stop listening to wifi p2p events
		mService.unregisterReceiver(mP2pEventReceiver);
		
		SharedPreferences prefs = mService.getSharedPreferences("dtnd", Context.MODE_PRIVATE);
		prefs.unregisterOnSharedPreferenceChangeListener(mPrefListener);
		
		// daemon goes down
		mServiceInfo = null;
		mServiceRequest = null;
		mWifiP2pManager = null;

		Intent i = new Intent(INTENT_P2P_STATE_CHANGED);
		i.putExtra(EXTRA_P2P_STATE, false);
		mService.sendBroadcast(i);
	}

	@Override
	public void connect(final String data) {
		if (mWifiP2pChannel == null) return;
		
		// get device object to connect to
		P2pDevice d = mDevices.get(data);
		
		// abort if the object is not available
		if (d == null) return;
		
		// do not proceed any action if there is an pending operation
		if (d.pending) return;
		
		// only connect if the state is AVAILABLE
		if (d.device.status != WifiP2pDevice.AVAILABLE) return;
		
		// connect to the peer identified by "data"
		WifiP2pConfig config = new WifiP2pConfig();
		config.deviceAddress = data;

		Log.d(TAG, "connect to " + d.endpoint.getString() + " (" + d.device.deviceAddress + ")");
		
		if (d.device.wpsPbcSupported()) {
			config.wps.setup = WpsInfo.PBC;
		} else if (d.device.wpsKeypadSupported()) {
			config.wps.setup = WpsInfo.KEYPAD;
		} else {
			config.wps.setup = WpsInfo.DISPLAY;
		}
		
		// set to invited
		d.pending = true;

		mWifiP2pManager.connect(mWifiP2pChannel, config, new WifiP2pManager.ActionListener() {
			@Override
			public void onFailure(int reason) {
				Log.e(TAG, "connection to " + data + " failed: " + reason);
				
				// update device list
				mWifiP2pManager.requestPeers(mWifiP2pChannel, mPeerListListener);
			}

			@Override
			public void onSuccess() {
				Log.d(TAG, "connection in progress to " + data);
				
				// update device list
				mWifiP2pManager.requestPeers(mWifiP2pChannel, mPeerListListener);
			}
		});
	}

	@Override
	public void disconnect(String data) {
		// get device object to connect to
		P2pDevice d = mDevices.get(data);
		
		// abort if the object is not available
		if (d == null) return;
		
		// disconnect from the peer identified by "data"
		Log.d(TAG, "disconnect from " + d.endpoint + " (" + d.device.deviceAddress + ")");
		
		// TODO: disconnect
	}

	private BroadcastReceiver mP2pEventReceiver = new BroadcastReceiver() {

		@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
		@Override
		public void onReceive(Context context, Intent intent) {
			Log.d(TAG, "P2P action: " + intent);

			String action = intent.getAction();

			if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
				int p2pState = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, WifiP2pManager.WIFI_P2P_STATE_DISABLED);

				if (p2pState == WifiP2pManager.WIFI_P2P_STATE_DISABLED) {
					Log.d(TAG, "Wi-Fi P2P has been disabled.");
					
					// switch to pause state
					pause();
				}
				else if (p2pState == WifiP2pManager.WIFI_P2P_STATE_ENABLED) {
					Log.d(TAG, "Wi-Fi P2P has been enabled.");

					// restore resumed state
					SharedPreferences prefs = mService.getSharedPreferences("dtnd", Context.MODE_PRIVATE);
					if (mResumed && prefs.getBoolean(Preferences.KEY_P2P_ENABLED, false)) resume();
				}
			}
			else if (mWifiP2pChannel != null) {
				if (WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
					int discoveryState = intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE,
							WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED);
	
					if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STOPPED) {
						Log.d(TAG, "Peer discovery has been stopped");
					}
					else if (discoveryState == WifiP2pManager.WIFI_P2P_DISCOVERY_STARTED) {
						Log.d(TAG, "Peer discovery has been started");
					}
				} else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
					if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
						WifiP2pDeviceList peers = (WifiP2pDeviceList) intent.getParcelableExtra(WifiP2pManager.EXTRA_P2P_DEVICE_LIST);
						handlePeersChanged(peers);
					} else {
						mWifiP2pManager.requestPeers(mWifiP2pChannel, mPeerListListener);
					}
				} else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
					connectionChanged(intent);
				}
			}
		}

		@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
		private void connectionChanged(Intent intent) {
			NetworkInfo netInfo = intent.getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);

			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
				// available since Android 4.3
				WifiP2pGroup p2pGroup = intent
						.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_GROUP);
				String iface = p2pGroup.getInterface();

				Log.d(TAG, "connection changed " + netInfo.toString() + " | " + p2pGroup.toString());

				if (netInfo.isConnected() && (iface != null)) {
					Log.d(TAG, "connection up " + iface);
					mInterfaces.add(iface);
					fireInterfaceUp(iface);
				} else if (mInterfaces.contains(iface)) {
					Log.d(TAG, "connection down " + iface);
					mInterfaces.remove(iface);
					fireInterfaceDown(iface);
				}
			} else if (netInfo.isConnected() && mWifiP2pChannel != null) {
				// request group info to get the interface of the group
				mWifiP2pManager.requestGroupInfo(mWifiP2pChannel, new GroupInfoListener() {
					@Override
					public void onGroupInfoAvailable(WifiP2pGroup group) {
						String iface = group.getInterface();
						Log.d(TAG, "connection up " + iface);

						// add the interface to the interface set
						mInterfaces.add(iface);

						// add the interface
						fireInterfaceUp(iface);
					}
				});
			}
			
			if (mWifiP2pChannel != null) {
				// update device list
				mWifiP2pManager.requestPeers(mWifiP2pChannel, mPeerListListener);
			}
		}
	};
	
	@SuppressLint("NewApi")
	public synchronized void startDiscovery() {
		mDiscoveryEnabled = true;
		if (mWifiP2pChannel == null) return;
		
		// add service discovery request
		mWifiP2pManager.addServiceRequest(mWifiP2pChannel, mServiceRequest, new WifiP2pManager.ActionListener() {
			@Override
			public void onFailure(int reason) {
				if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
					Log.e(TAG,
							"failed to add service request: Wi-Fi Direct is not supported by this device!");
				}
				else {
					Log.e(TAG, "failed to add service request: " + reason);
				}
			}

			@Override
			public void onSuccess() {
				Log.d(TAG, "service request added");
				
				// start service discovery
				mWifiP2pManager.discoverServices(mWifiP2pChannel, new WifiP2pManager.ActionListener() {
					@Override
					public void onFailure(int reason) {
						if (reason == WifiP2pManager.P2P_UNSUPPORTED) {
							Log.e(TAG, "starting service discovery failed: Wi-Fi Direct is not supported by this device!");
						}
						else if (reason == WifiP2pManager.NO_SERVICE_REQUESTS) {
							Log.e(TAG, "starting service discovery failed: No service requests!");
						}
						else {
							Log.e(TAG, "starting service discovery failed: " + reason);
						}
					}
		
					@Override
					public void onSuccess() {
						Log.d(TAG, "service discovery started");
					}
				});
			}
		});
	}

	public synchronized void stopDiscovery() {
		mDiscoveryEnabled = false;
		if (mWifiP2pChannel == null) return;
		
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
	}

	private PeerListListener mPeerListListener = new PeerListListener() {
		@Override
		public void onPeersAvailable(WifiP2pDeviceList peers) {
			handlePeersChanged(peers);
		}
	};
	
	private void handlePeersChanged(WifiP2pDeviceList peers) {
		Log.d(TAG, "Peers available: " + peers.getDeviceList().size());
		HashSet<String> discoveredDevices = new HashSet<String>();
		
		for (WifiP2pDevice peer : peers.getDeviceList()) {
			P2pDevice d = getDevice(peer.deviceAddress);
			
			d.device = peer;
			d.pending = false;
			Log.d(TAG, "Peer: " + peer.deviceName + " (" + peer.deviceAddress + ", state: " + peer.status + ")");
			
			// add to list of discovered devices
			discoveredDevices.add(d.device.deviceAddress);
			
			if (d.endpoint != null) {
				fireConnected(d.endpoint, d.device.deviceAddress, 120);
			}
		}
		
		Set<String> knownDevices = mDevices.keySet();
		knownDevices.removeAll(discoveredDevices);
		
		// remove all disappeared devices
		for (String deviceAddress : knownDevices) {
			P2pDevice d = mDevices.get(deviceAddress);
			if (d.endpoint != null) {
				fireDisconnected(d.endpoint, deviceAddress);
			}
			mDevices.remove(deviceAddress);
		}
	}
	
	private P2pDevice getDevice(String deviceAddress) {
		P2pDevice d = mDevices.get(deviceAddress);
		
		if (d == null) {
			d = new P2pDevice();
			mDevices.put(deviceAddress, d);
		}
		
		return d;
	}

	private DnsSdTxtRecordListener mRecordListener = new DnsSdTxtRecordListener() {
		@Override
		public void onDnsSdTxtRecordAvailable(String fullDomainName,
				Map<String, String> txtRecordMap, WifiP2pDevice srcDevice) {
			Log.d(TAG,
					"DnsSdTxtRecord discovered: " + fullDomainName + ", "
							+ txtRecordMap.toString());
			
			P2pDevice d = getDevice(srcDevice.deviceAddress);

			d.device = srcDevice;
			d.endpoint = new EID(txtRecordMap.get("eid"));
			
			fireDiscovered(d.endpoint, d.device.deviceAddress, 60);
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
			ArrayList<NetworkInterface> ilist = Collections.list(NetworkInterface.getNetworkInterfaces());

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
