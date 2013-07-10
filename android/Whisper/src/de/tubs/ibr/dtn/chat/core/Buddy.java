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

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.chat.RosterAdapter;

public class Buddy implements Comparable<Buddy> {
	
	private static final String TAG = "Buddy";

	public static final String ID = BaseColumns._ID;
	public static final String NICKNAME = "nickname";
	public static final String ENDPOINT = "endpoint";
	public static final String LASTSEEN = "lastseen";
	public static final String PRESENCE = "presence";
	public static final String STATUS = "status";
	public static final String DRAFTMSG = "draftmsg";
	public static final String VOICEEID = "voiceeid";
	public static final String LANGUAGE = "language";
	
	private Long id = null;
	private String nickname = null;
	private String endpoint = null;
	private Date lastseen = null;
	private String presence = null;
	private String status = null;
	private String draftmsg = null;
	private String voiceeid = null;
	private String language = null;

	public Buddy(Context context, Cursor cursor, RosterAdapter.ColumnsMap cmap)
	{
		final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

		this.id = cursor.getLong(cmap.mColumnId);
		this.nickname = cursor.getString(cmap.mColumnNickname);
		this.endpoint = cursor.getString(cmap.mColumnEndpoint);
		this.lastseen = null;
		this.presence = cursor.getString(cmap.mColumnPresence);
		this.status = cursor.getString(cmap.mColumnStatus);
		this.draftmsg = cursor.getString(cmap.mColumnDraftMsg);
		this.voiceeid = cursor.getString(cmap.mColumnVoiceEndpoint);
		this.language = cursor.getString(cmap.mColumnLanguage);
		
		// set the last seen parameter
		if (!cursor.isNull(cmap.mColumnLastseen))
		{
			try {
				this.lastseen = formatter.parse( cursor.getString(cmap.mColumnLastseen) );
			} catch (ParseException e) {
				Log.e(TAG, "failed to convert date: " + cursor.getString(cmap.mColumnLastseen));
			}
		}
	}

	public Long getId() {
		return id;
	}

	public void setId(Long id) {
		this.id = id;
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
	
	public int compareTo(Buddy another) {
		if (isOnline() == another.isOnline())
			return this.toString().compareToIgnoreCase(another.toString());
		
		if (isOnline()) return -1;
		
		return 1;
	}

	public String toString() {
		if (this.nickname != null)
			return this.nickname;
		
		if (this.endpoint != null)
			return this.endpoint;
		
		return super.toString();
	}
	
	public Boolean hasVoice() {
	    return (voiceeid != null);
	}
	
    public String getVoiceEndpoint() {
        return voiceeid;
    }

    public String getLanguage() {
        return language;
    }

    public void setLanguage(String language) {
        this.language = language;
    }
}
