package de.tubs.ibr.dtn.p2p.db;

import java.util.Calendar;
import java.util.Date;

import android.database.Cursor;
import android.provider.BaseColumns;

public class Peer {
    
    @SuppressWarnings("unused")
    private static final String TAG = "Peer";
    
    public static final String ID = BaseColumns._ID;
    public static final String P2P_ADDRESS = "address";
    public static final String ENDPOINT = "endpoint";
    public static final String LAST_SERVICE_DISCO = "last_service_disco";
    public static final String LAST_SEEN = "last_seen";
    public static final String CONNECT_STATE = "connect_state";
    public static final String CONNECT_UPDATE = "connect_update";

    private String mP2pAddress = null;
    private String mEndpoint = null;
    private Date mLastServiceDisco = null;
    private Date mLastSeen = null;
    private Integer mConnectState = 0;
    private Date mConnectUpdate = null;
    
    public Peer(Cursor cursor, PeerAdapter.ColumnsMap cmap) {
        mP2pAddress = cursor.getString(cmap.mColumnP2pAddress);
        mEndpoint = cursor.getString(cmap.mColumnEndpoint);
        mLastServiceDisco = new Date(cursor.getLong(cmap.mColumnLastServiceDisco));
        mLastSeen = new Date(cursor.getLong(cmap.mColumnLastSeen));
        mConnectState = cursor.getInt(cmap.mColumnConnectState);
        mConnectUpdate = new Date(cursor.getLong(cmap.mColumnConnectUpdate));
    }

    public Peer(String addr) {
        super();
        this.mP2pAddress = addr;
        this.mEndpoint = null;
        this.mLastSeen = new Date();
        this.mConnectState = 0;
    }

    public String getP2pAddress() {
        return mP2pAddress;
    }

    public void setP2pAddress(String p2pAddress) {
        mP2pAddress = p2pAddress;
    }

    public String getEndpoint() {
        return mEndpoint;
    }

    public void setEndpoint(String endpoint) {
        mEndpoint = endpoint;
        mLastServiceDisco = new Date();
    }
    
    public boolean hasEndpoint() {
        return (mEndpoint != null);
    }
    
    public boolean isEndpointValid() {
        if (mEndpoint == null) return false;
        if (mLastServiceDisco == null) return false;
        
        Calendar timeframe = Calendar.getInstance();
        timeframe.add(Calendar.MINUTE, -30);
        
        if (mLastServiceDisco.before(timeframe.getTime())) return false;
        
        return true;
    }
    
    public Date getLastServiceDisco() {
        return mLastServiceDisco;
    }

    public Date getLastSeen() {
        return mLastSeen;
    }
    
    public void touch() {
        mLastSeen = new Date();
    }

    public Integer getConnectState() {
        if (mConnectState > 1) {
            if (mConnectUpdate == null) return 1;
            
            Calendar one_minute_ago = Calendar.getInstance();
            one_minute_ago.add(Calendar.MINUTE, -1);
            
            if (mConnectUpdate.before(one_minute_ago.getTime())) return 1;
        }

        return mConnectState;
    }

    public void setConnectState(Integer connectState) {
        mConnectUpdate = new Date();
        mConnectState = connectState;
    }

    public Date getConnectUpdate() {
        return mConnectUpdate;
    }

    @Override
    public String toString() {
        if (mEndpoint != null) {
            return String.format("%s [seen: %d, state: %d]", mP2pAddress, mLastSeen.getTime(), mConnectState);
        } else {
            return String.format("%s alias %s [seen: %d, state: %d]", mP2pAddress, mEndpoint, mLastSeen.getTime(), mConnectState);
        }
    }
}
