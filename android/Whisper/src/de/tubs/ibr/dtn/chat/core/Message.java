/*
 * Message.java
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

import java.util.Date;

public class Message {

	private Long msgid = null;
	private Boolean incoming = null;
	private Buddy buddy = null;
	private Date created = null;
	private Date received = null;
	private String payload = null;
	private String sentid = null;
	private Long flags = 0L;
	
	public Message(Long id, Boolean incoming, Date created, Date received, String payload)
	{
		this.msgid = id;
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
	public void setSentId(String id) {
		this.sentid = id;
	}
	public String getSentId() {
		return this.sentid;
	}

	public Long getMsgId() {
		return msgid;
	}

	public void setMsgId(Long msgid) {
		this.msgid = msgid;
	}

	public Long getFlags() {
		return flags;
	}

	public void setFlags(Long flags) {
		this.flags = flags;
	}
}
