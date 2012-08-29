package de.tubs.ibr.dtn.api;

import de.tubs.ibr.dtn.api.DTNClient.Session;

public interface SessionConnection {
    /**
     * On every successful registration the sessionConnected() method is called.
     * @param session The new initialized session.
     */
	public void onSessionConnected(Session session);
	
	public void onSessionDisconnected();
}
