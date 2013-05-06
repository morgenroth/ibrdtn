package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
import android.content.Intent;
import android.util.Log;
import de.tubs.ibr.dtn.service.DaemonService;
import de.tubs.ibr.dtn.swig.EID;
import de.tubs.ibr.dtn.swig.NativeP2pManager;

@TargetApi(16)
public class P2PManager extends NativeP2pManager {
	
	private final static String TAG = "P2PManager";
	
	private DaemonService mService = null;
	
	/**
	 * How to control the P2P API of the daemon
	 * 
	 * The following control calls are only possible if the client is connected to the
	 * daemon. That means, do such a call only if "initialize" was successful and
	 * "destroy" has not been called yet.
	 * 
	 * Call a "fireDiscovered" event for each discovered peer earlier than
	 * every 120 seconds.
	 * fireDiscovered(new ibrdtn.api.object.SingletonEndpoint("dtn://test-node"), data);
	 *  
	 * If a new interface has been created due to a P2P connection, then call the
	 * "fireInterfaceUp" event.
	 * fireInterfaceUp("wlan0-p2p0");
	 * 
	 * If a P2P interface goes down then call "fireInterfaceDown".
	 * fireInterfaceDown("wlan0-p2p0");
	 *
	 */
	
	public P2PManager(DaemonService service) {
	    super("P2P:WIFI");
	    this.mService = service;
	}
	
	public void initialize() {
	    // daemon is up
	    
	    // bind to service?
	}

	public void destroy() {
	    // daemon goes down - stop all actions
	}
	
    public void fireInterfaceUp(String iface) {
        super.fireInterfaceUp(iface);
    }
	
	public void fireInterfaceDown(String iface) {
	    super.fireInterfaceDown(iface);
	}
	
	public void fireDiscovered(String eid, String identifier) {
	    Log.d(TAG, "found peer: " + eid + ", " + identifier);
	    super.fireDiscovered(new EID(eid), identifier, 90, 10);
	}

    @Override
    public void connect(String data) {
        // connect to the peer identified by "data"
        Intent i = new Intent(
        		"de.hendrikfreytag.wifip2p4ibrdtn.action.CONNECT_TO_PEER");
        i.putExtra("de.hendrikfreytag.wifip2p4ibrdtn.extra.MAC", data);
        mService.sendBroadcast(i);
        Log.i(TAG, "connect request: " + data);
    }

    @Override
    public void disconnect(String data) {
        // disconnect from the peer identified by "data"
        Intent i = new Intent(
        		"de.hendrikfreytag.wifip2p4ibrdtn.action.DISCONNECT_FROM_PEER");
        i.putExtra("de.hendrikfreytag.wifip2p4ibrdtn.extra.MAC", data);
        mService.sendBroadcast(i);
        Log.i(TAG, "disconnect request: " + data);
    }
}
