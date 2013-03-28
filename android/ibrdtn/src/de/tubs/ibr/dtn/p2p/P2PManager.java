package de.tubs.ibr.dtn.p2p;

//import ibrdtn.api.APIConnection;
//import ibrdtn.api.P2PEventListener;
//import ibrdtn.api.P2PExtensionClient;
//import ibrdtn.api.P2PExtensionClient.ExtensionType;

import java.io.IOException;
import java.net.UnknownHostException;

import android.annotation.TargetApi;
import android.content.Context;
import android.net.wifi.WifiManager;
import android.util.Log;

@TargetApi(16)
public class P2PManager {
	
	private final static String TAG = "P2PManager";
	
	private WifiManager _wifi_manager = null;
	private WifiManager.MulticastLock _mcast_lock = null;
//	private P2PExtensionClient _p2pclient = null;
	private Context _context = null;
	private String _eid = null;
	
	/**
	 * How to control the P2P API of the daemon
	 * 
	 * The following control calls are only possible if the client is connected to the
	 * daemon. That means, do such a call only if "initialize" was successful and
	 * "destroy" has not been called yet.
	 * 
	 * Call a "fireDiscovered" event for each discovered peer earlier than
	 * every 120 seconds.
	 * _p2pclient.fireDiscovered(new ibrdtn.api.object.SingletonEndpoint("dtn://test-node"), data);
	 *  
	 * If a new interface has been created due to a P2P connection, then call the
	 * "fireInterfaceUp" event.
	 * _p2pclient.fireInterfaceUp("wlan0-p2p0");
	 * 
	 * If a P2P interface goes down then call "fireInterfaceDown".
	 * _p2pclient.fireInterfaceDown("wlan0-p2p0");
	 *
	 */
	
	public P2PManager(Context context, String eid) {
		this._context = context;
		this._eid = eid;
	}

//	public void initialize(APIConnection conn) {
//		_wifi_manager = (WifiManager)_context.getSystemService(Context.WIFI_SERVICE);
//		
//		// listen to multicast packets
//		_mcast_lock = _wifi_manager.createMulticastLock(TAG);
//		_mcast_lock.acquire();
//		
//		// check the API connection
//		if (conn == null) return;
//		
//		Log.i(TAG, "initialized, own EID is " + this._eid);
//		
//		// create a new P2P client
//		_p2pclient = new P2PExtensionClient(ExtensionType.WIFI);
//		_p2pclient.setConnection( conn );
//		_p2pclient.setP2PEventListener(_listener);
//		
//		try {
//			_p2pclient.open();
//			Log.i(TAG, "API connected");
//		} catch (UnknownHostException e) {
//			e.printStackTrace();
//		} catch (IOException e) {
//			e.printStackTrace();
//		}
//	}
	
	public void destroy() {
//		try {
//			Log.i(TAG, "closing API connection");
//			_p2pclient.close();
//		} catch (IOException e) {
//			e.printStackTrace();
//		}
//		_p2pclient = null;
//		
//		if (_mcast_lock != null) {
//			_mcast_lock.release();
//		}
//		_wifi_manager = null;
	}
	
//	private P2PEventListener _listener = new P2PEventListener() {
//		@Override
//		public void connect(String data) {
//			// connect to the peer identified by "data"
//			Log.i(TAG, "connect request: " + data);
//		}
//
//		@Override
//		public void disconnect(String data) {
//			// disconnect from the peer identified by "data"
//			Log.i(TAG, "disconnect request: " + data);
//		}
//	};
}
