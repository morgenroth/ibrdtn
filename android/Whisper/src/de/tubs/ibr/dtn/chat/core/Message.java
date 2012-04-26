package de.tubs.ibr.dtn.chat.core;

import java.util.Date;

public class Message {

	private Boolean incoming = null;;
	private Buddy buddy = null;
	private Date created = null;;
	private Date received = null;;
	private String payload = null;;
	
	public Message(Boolean incoming, Date created, Date received, String payload)
	{
		this.incoming = incoming;
		this.created = created;
		this.received = received;
		this.payload = payload;
	}
	
	public Buddy getBuddy() {
		return buddy;
	}

	public void setBuddy(Buddy buddy) {
		this.buddy = buddy;
	}
	
	public Boolean isIncoming() {
		return incoming;
	}
	public void setIncoming(Boolean incoming) {
		this.incoming = incoming;
	}
	public Date getCreated() {
		return created;
	}
	public void setCreated(Date created) {
		this.created = created;
	}
	public Date getReceived() {
		return received;
	}
	public void setReceived(Date received) {
		this.received = received;
	}
	public String getPayload() {
		return payload;
	}
	public void setPayload(String payload) {
		this.payload = payload;
	}
	
}
