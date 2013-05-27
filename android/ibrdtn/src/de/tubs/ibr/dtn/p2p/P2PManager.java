package de.tubs.ibr.dtn.p2p;

import android.annotation.TargetApi;
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

    @Override
    public void connect(String identifier) {
        // now try to connect to the peer identified by the "identifier"
        
        EID eid = new EID("dtn://the-other-peer");
        long timeout = 120L;
        int priority = 20;

        // if the connection was successful, then fire the connected event
        this.fireConnected(eid, identifier, timeout, priority);
    }

    @Override
    public void disconnect(String identifier) {
        // now disconnect from the peer identified by the "identifier"
        
        EID eid = new EID("dtn://the-other-peer");

        // if the connection was successful, then fire the connected event
        this.fireDisconnected(eid, identifier);
    }
}
