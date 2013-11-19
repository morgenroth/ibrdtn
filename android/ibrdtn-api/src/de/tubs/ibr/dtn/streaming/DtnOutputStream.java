package de.tubs.ibr.dtn.streaming;

import de.tubs.ibr.dtn.api.DTNClient;
import de.tubs.ibr.dtn.api.EID;

public class DtnOutputStream {
    
    private DTNClient.Session mSession = null;
    private int mCorrelator = 0;
    private EID mDestination = null;
    private long mLifetime = 3600L;
    
    public DtnOutputStream(DTNClient.Session session, int correlator, EID destination, long lifetime) {
        mCorrelator = correlator;
        mSession = session;
        mDestination = destination;
        mLifetime = lifetime;
    }

    /**
     * write a frame into the packet stream
     * @param data
     */
    public void put(byte[] data) {
        
    }
    
    /**
     * Close the packet stream
     * (transmits a FIN)
     */
    public void close() {
        
    }
}
