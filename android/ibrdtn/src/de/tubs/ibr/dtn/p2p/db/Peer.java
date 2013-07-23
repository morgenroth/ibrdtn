package de.tubs.ibr.dtn.p2p.db;

import java.text.DateFormat;
import java.util.Date;

import android.database.Cursor;
import android.provider.BaseColumns;

public class Peer {
    
    @SuppressWarnings("unused")
    private static final String TAG = "Peer";
    
    public static final String ID = BaseColumns._ID;
    public static final String MAC_ADDRESS = "mac";
    public static final String EID = "eid";
    public static final String LAST_CONTACT = "last_contact";
    public static final String EVER_CONNECTED = "ever_connected";

    private String mMacAddress = null;
    private String mEid = null;;
    private Date mLastContact = null;;
    private boolean mEverConnected = false;
    
    public Peer(Cursor cursor, PeerAdapter.ColumnsMap cmap) {
        mMacAddress = cursor.getString(cmap.mColumnMacAddress);
        mEid = cursor.getString(cmap.mColumnEid);
        mLastContact = new Date(cursor.getLong(cmap.mColumnLastContact));
        mEverConnected = cursor.getInt(cmap.mColumnEverConnected) == 1;
    }

    public Peer(String macAddress, String eid, Date lastContact,
            boolean everConnected) {
        super();
        this.mMacAddress = macAddress;
        this.mEid = eid;
        this.mLastContact = lastContact;
        this.mEverConnected = everConnected;
    }

    public String getMacAddress() {
        return mMacAddress;
    }

    public void setMacAddress(String macAddress) {
        this.mMacAddress = macAddress;
    }

    public Date getLastContact() {
        return mLastContact;
    }

    public void setLastContact(Date lastContact) {
        this.mLastContact = lastContact;
    }

    public boolean isEverConnected() {
        return mEverConnected;
    }

    public void setEverConnected(boolean everConnected) {
        this.mEverConnected = everConnected;
    }

    public String getEid() {
        return mEid;
    }

    public void setEid(String eid) {
        this.mEid = eid;
    }

    public boolean hasEid() {
        return mEid.length() > 0;
    }

    @Override
    public String toString() {
        return String.format("%s,%s,%s,%b", mMacAddress, mEid, DateFormat
                .getDateInstance().format(mLastContact), mEverConnected);
    }
}
