package de.tubs.ibr.dtn.chat.core;

import java.util.Calendar;
import java.util.Date;

public class Buddy implements Comparable<Buddy> {

	private String nickname = null;
	private String endpoint = null;
	private Date lastseen = null;
	private String presence = null;
	private String status = null;

	public Buddy(Roster roster, String nickname, String endpoint, String presence, String status)
	{
		this.nickname = nickname;
		this.endpoint = endpoint;
		this.lastseen = null;
		this.presence = presence;
		this.status = status;
	}

	public Date getLastSeen() {
		return lastseen;
	}

	public void setLastSeen(Date lastseen) {
		this.lastseen = lastseen;
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
	
	public Date getExpiration()
	{
		Calendar cal = Calendar.getInstance();
		cal.add(Calendar.HOUR, 1);
		cal.add(Calendar.MINUTE, 1);
		return cal.getTime();
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
