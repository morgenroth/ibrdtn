package de.tubs.ibr.dtn.chat;

import de.tubs.ibr.dtn.chat.core.Roster;
import de.tubs.ibr.dtn.chat.service.ChatServiceHelper.ServiceNotConnectedException;

public interface RosterProvider {
	public Roster getRoster() throws ServiceNotConnectedException;
}
