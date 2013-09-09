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
package de.tubs.ibr.dtn.dtalkie.db;

import java.io.File;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.content.Context;
import android.database.Cursor;
import android.provider.BaseColumns;
import android.util.Log;
import de.tubs.ibr.dtn.dtalkie.MessageAdapter;

@SuppressLint("SimpleDateFormat")
public class Message implements Comparable<Message> {
	
    private final static String TAG = "Message";
    
    public static final String ID = BaseColumns._ID;
    public static final String SOURCE = "source";
    public static final String DESTINATION = "destination";
    public static final String RECEIVED = "received";
    public static final String CREATED = "created";
    public static final String FILENAME = "filename";
    public static final String PRIORITY = "priority";
    public static final String MARKED = "marked";
    
	private Long mId = null;
	private String mSource = null;
	private String mDestination = null;
	private Date mReceived = null;
	private Date mCreated = null;
	private File mFile = null;
	private Long mPriority = null;
	private Boolean mMarked = null;
	
    public Message(Context context, Cursor cursor, MessageAdapter.ColumnsMap cmap)
    {
        final DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        mId = cursor.getLong(cmap.mColumnId);
        mSource = cursor.getString(cmap.mColumnSource);
        mDestination = cursor.getString(cmap.mColumnDestination);
        
        try {
            this.mCreated = formatter.parse(cursor.getString(cmap.mColumnCreated));
            this.mReceived = formatter.parse(cursor.getString(cmap.mColumnReceived));
        } catch (ParseException e) {
            Log.e(TAG, "failed to convert date");
        }
        
        mFile = new File(cursor.getString(cmap.mColumnFilename));
        
        mPriority = cursor.getLong(cmap.mColumnPriority);
        mMarked = "yes".equals(cursor.getString(cmap.mColumnMarked));
    }
	
	public Message(String source, String destination, File file) {
	    mSource = source;
	    mDestination = destination;
	    mFile = file;
	    mCreated = new Date();
	}
	
	public String getSource() {
		return mSource;
	}
	
	public void setSource(String source) {
		this.mSource = source;
	}
	
	public String getDestination() {
		return mDestination;
	}
	
	public void setDestination(String destination) {
		this.mDestination = destination;
	}

	public File getFile() {
		return mFile;
	}

	public void setFile(File file) {
		this.mFile = file;
	}

	public Boolean isMarked() {
		if (mMarked == null) return false;
		return mMarked;
	}

	public void setMarked(Boolean marked) {
		this.mMarked = marked;
	}

	public Long getPriority() {
		return mPriority;
	}

	public void setPriority(Long priority) {
		this.mPriority = priority;
	}

	public Date getCreated() {
		return mCreated;
	}

	public void setCreated(Date created) {
		this.mCreated = created;
	}

	public Date getReceived() {
		return mReceived;
	}

	public void setReceived(Date received) {
		this.mReceived = received;
	}

	public Long getId() {
		return mId;
	}

	public int compareTo(Message arg0) {
		if (this.mCreated != null)
		{
			if (!this.mCreated.equals(arg0.mCreated))
				return this.mCreated.compareTo(arg0.mCreated);
		}
		
		if (this.mReceived != null)
		{
			if (!this.mReceived.equals(arg0.mCreated))
				return this.mReceived.compareTo(arg0.mCreated);
		}
		
		if (this.mSource != null)
		{
			if (!this.mSource.equals(arg0.mSource))
				return this.mSource.compareTo(arg0.mSource);
		}
		
		if (this.mPriority != null)
		{
			if (!this.mPriority.equals(arg0.mPriority))
				return this.mPriority.compareTo(arg0.mPriority);
		}
		
		if (this.mFile != null)
		{
			if (!this.mFile.equals(arg0.mFile))
				return this.mFile.compareTo(arg0.mFile);
		}

		return 0;
	}
}
