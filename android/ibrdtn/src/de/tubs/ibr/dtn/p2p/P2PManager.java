package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
import android.content.Context;
import android.net.wifi.WifiManager;

@TargetApi(16)
public class P2PManager {
	
	private final static String TAG = "P2PManager";
	
	private WifiManager _wifi_manager = null;
	private WifiManager.MulticastLock _mcast_lock = null;
//	private WifiP2pManager _wifi_p2p_manager = null;
	private Context _context = null;
	private P2PNeighborListener _listener = null;
	private String _eid = null;
	
	public interface P2PNeighborListener {
		public void onNeighborDetected(String name);
		public void onNeighborDisappear(String name);
		public void onNeighborConnected(String name, String iface);
		public void onNeighborDisconnected(String name, String iface);
	}
	
	public P2PManager(Context context, P2PNeighborListener listener, String eid) {
		this._context = context;
		this._listener = listener;
		this._eid = eid;
	}

	public void initialize() {
		_wifi_manager = (WifiManager)_context.getSystemService(Context.WIFI_SERVICE);
		
		// listen to multicast packets
		_mcast_lock = _wifi_manager.createMulticastLock(TAG);
		_mcast_lock.acquire();
		
//		_wifi_p2p_manager = (WifiP2pManager)_context.getSystemService(Context.WIFI_P2P_SERVICE);
//		_wifi_p2p_manager.initialize(this,  null, _action_listener);
		
	}
	
	public void destroy() {
//		_wifi_p2p_manager = null;
		if (_mcast_lock != null) {
			_mcast_lock.release();
		}
		_wifi_manager = null;
	}
	
//	private WifiP2pManager.ActionListener _action_listener = new  WifiP2pManager.ActionListener() {
//
//		@Override
//		public void onFailure(int reason) {
//			Log.e(TAG, "wifi-direct is not available");
//		}
//
//		@Override
//		public void onSuccess() {
//			Map<String, String> txtMap = null;
//			WifiP2pServiceInfo info = WifiP2pDnsSdServiceInfo.newInstance(_eid, "bundle", txtMap);
//		}
//		
//	};
}
