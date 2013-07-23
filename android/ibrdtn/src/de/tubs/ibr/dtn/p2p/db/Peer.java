package de.tubs.ibr.dtn.p2p.db;

import java.text.DateFormat;
import java.util.Date;

public class Peer {

    private String macAddress;
    private String eid;
    private Date lastContact;
    private boolean everConnected;

    public Peer(String macAddress, String eid, Date lastContact,
            boolean everConnected) {
        super();
        this.macAddress = macAddress;
        this.eid = eid;
        this.lastContact = lastContact;
        this.everConnected = everConnected;
    }

    public String getMacAddress() {
        return macAddress;
    }

    public void setMacAddress(String macAddress) {
        this.macAddress = macAddress;
    }

    public Date getLastContact() {
        return lastContact;
    }

    public void setLastContact(Date lastContact) {
        this.lastContact = lastContact;
    }

    public boolean isEverConnected() {
        return everConnected;
    }

    public void setEverConnected(boolean everConnected) {
        this.everConnected = everConnected;
    }

    public String getEid() {
        return eid;
    }

    public void setEid(String eid) {
        this.eid = eid;
    }

    public boolean hasEid() {
        return eid.length() > 0;
    }

    @Override
    public String toString() {
        return String.format("%s,%s,%s,%b", macAddress, eid, DateFormat
                .getDateInstance().format(lastContact), everConnected);
    }

}
