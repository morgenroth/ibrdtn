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

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.chat.MessageAdapter;

public class Message {
	
	private static final String TAG = "Message";

	public static final String ID = BaseColumns._ID;
	public static final String BUDDY = "buddy";
	public static final String DIRECTION = "direction";
	public static final String CREATED = "created";
	public static final String RECEIVED = "received";
	public static final String PAYLOAD = "payload";
	public static final String SENTID = "sentid";
	public static final String FLAGS = "flags";
	
	public static final int FLAG_SENT = 1;
	public static final int FLAG_DELIVERED = 2;
	public static final int FLAG_SIGNED = 4;
	public static final int FLAG_ENCRYPTED = 8;
	
	private Long msgid = null;
	private Boolean incoming = null;
	private Long buddyid = null;
	private Date created = null;
	private Date received = null;
	private String payload = null;
	private String sentid = null;
	private Long flags = 0L;
	
	public Message(Context context, Cursor cursor, MessageAdapter.ColumnsMap cmap)
	{
		final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		
		this.msgid = cursor.getLong(cmap.mColumnId);
		this.buddyid = cursor.getLong(cmap.mColumnBuddy);
		this.incoming = cursor.getString(cmap.mColumnDirection).equals("in");
		
		try {
			this.created = formatter.parse(cursor.getString(cmap.mColumnCreated));
			this.received = formatter.parse(cursor.getString(cmap.mColumnReceived));
		} catch (ParseException e) {
			Log.e(TAG, "failed to convert date");
		}
		
		this.payload = cursor.getString(cmap.mColumnPayload);
		
		this.sentid = cursor.getString(cmap.mColumnSendId);
		this.flags = cursor.getLong(cmap.mColumnFlags);
	}
	
	public Long getBuddyId() {
		return buddyid;
	}

	public void setBuddyId(Long buddyid) {
		this.buddyid = buddyid;
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
		return (flags == null) ? 0 : flags;
	}

	public void setFlags(Long flags) {
		this.flags = flags;
	}
	
	public boolean hasFlag(int flag) {
		return (this.flags & flag) > 0;
	}
	
	public void setFlag(int flag, boolean value) {
		if (value) {
			this.flags |= flag;
		} else {
			this.flags &= ~flag;
		}
	}
}
