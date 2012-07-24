/*
 * Buddy.java
 * 
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
package de.tubs.ibr.dtn.chat.core;

import java.util.Calendar;
import java.util.Date;

public class Buddy implements Comparable<Buddy> {

	private String nickname = null;
	private String endpoint = null;
	private Date lastseen = null;
	private String presence = null;
	private String status = null;
	private String draftmsg = null;

	public Buddy(String nickname, String endpoint, String presence, String status, String draftmsg)
	{
		this.nickname = nickname;
		this.endpoint = endpoint;
		this.lastseen = null;
		this.presence = presence;
		this.status = status;
		this.draftmsg = draftmsg;
	}

	public Date getLastSeen() {
		return lastseen;
	}

	public void setLastSeen(Date lastseen) {
		this.lastseen = lastseen;
	}
	
	public String getDraftMessage() {
		return this.draftmsg;
	}
	
	public void setDraftMessage(String msg) {
		this.draftmsg = msg;
	}
	
	public Boolean hasDraft() {
		return (this.draftmsg != null);
	}
	
	public Boolean isOnline()
	{
		Calendar cal = Calendar.getInstance();
		cal.add(Calendar.HOUR, -1);
		
		if (getLastSeen() == null)
		{
			return false;
		}
		else  if ( cal.getTime().after(getLastSeen()) )
		{
			return false;
		}
		
		return true;
	}
	
	public Calendar getExpiration()
	{
		Calendar cal = Calendar.getInstance();
		cal.add(Calendar.HOUR, 1);
		return cal;
	}
	
	public String getPresence() {
		return presence;
	}

	public void setPresence(String presence) {
		this.presence = presence;
	}

	public String getStatus() {
		return status;
	}

	public void setStatus(String status) {
		this.status = status;
	}

	public String getEndpoint() {
		return endpoint;
	}

	public void setEndpoint(String endpoint) {
		this.endpoint = endpoint;
	}

	public String getNickname() {
		return nickname;
	}

	public void setNickname(String nickname) {
		this.nickname = nickname;
	}
	
	@Override
	public int compareTo(Buddy another) {
		if (isOnline() == another.isOnline())
			return this.toString().compareToIgnoreCase(another.toString());
		
		if (isOnline()) return -1;
		
		return 1;
	}

	@Override
	public String toString() {
		if (this.nickname != null)
			return this.nickname;
		
		if (this.endpoint != null)
			return this.endpoint;
		
		return super.toString();
	}
}
